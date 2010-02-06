
/* main - main file
 *
 * Copyright (C) 2009-2010 Kingstone, ltd
 *
 * Written by microcai in 2009-2010
 *
 * This software is lisenced under the Kingstone mid-ware Lisence.
 *
 * For more infomation see COPYING file shipped with this software.
 *
 * If you have any question with this software, please contract microcai, the
 * original writer of this software.
 *
 * If you have any question with law suite, please contract 黄小克, the owner of
 * this company.
 *
 */

#ifdef HAVE_CONFIG_H
#include  "config.h"
#endif

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <glib.h>
#include <glib-object.h>

#include "inifile.h"
#include "prase.h"

#include "../include/global_var.h"
#include "../include/distdb.h"

static int nodaemon,setdaemon=1;
static char	config_file[256]=CONF_FILE;
static int	showversion;

static struct parameter_tags param[] =
{
	{"-D", (char*)&nodaemon," -D\t\t DO NOT fork as a deamon",sizeof(nodaemon),2, BOOL_both},
	{"--daemon", (char*)&setdaemon,"    --daemon\t run as a daemon(default)",sizeof(setdaemon),8, BOOL_both},
	{"-f",config_file,0,sizeof(config_file),2,STRING},
	{"--config",config_file," -f,--config\t specify alternative config file",sizeof(config_file),8,STRING},
	{"--version", (char*)&showversion ,"    --version\t show the version of RuijieClient",sizeof(showversion),9, BOOL_both},
	{0}
};


struct cfg_progs
{
	/*
	 * node file that use to connect
	 */
	char * startup_node_file;

	/*
	 * url for the node file to get, distdb will download and use the new file
	 * only of the file in the server is newer or local file collapse.
	 */
	char * url_nodefile;
}cfg_progs;

typedef struct _NODES{
	struct sockaddr_in peer;
	int group;
}NODE;


static 	struct distdb_info distdbinfo = {0};
static  GList * node_fromfile;

static int getconfig(struct cfg_progs * cfg,int argc, char * argv[])
{
	char	keyval[128];
	ParseParameters(&argc,&argv,param);
	if (showversion)
	{
		printf("%s\n",VERSION);
		exit(EXIT_SUCCESS);
	}
	FILE * cfile = fopen(config_file,"r");
	if (!cfile)
	{
		fprintf(stderr,"Can not open %s.!\n",config_file);
		exit(EXIT_FAILURE);
	}
	if (cfg->startup_node_file)
		free((void*)cfg->startup_node_file);
	cfg->startup_node_file = malloc(1024);
	if (cfg->url_nodefile)
		free(cfg->url_nodefile);
	cfg->url_nodefile = malloc(8192);

	get_profile_string(cfile,"global","nodes_file",cfg->startup_node_file,1024);
	get_profile_string(cfile,"global","nodes_url",cfg->url_nodefile,8192);
	get_profile_string(cfile,"global","node_name",(char*)distdbinfo.servername,8192);
	get_profile_string(cfile,"global","backend",keyval,8192);
	if(strcmp("sqlite",keyval)==0)
	{
		distdbinfo.backend = 0;
	}else if(strcmp("mysql",keyval)==0)
	{
		distdbinfo.backend = 1;
	}
	else if (strcmp("oracle", keyval) == 0)
	{
		distdbinfo.backend = 2;

	}
	else if (strcmp("oci", keyval) == 0)
	{
		distdbinfo.backend = 2;
	}
	else
	{
		perror("非法的数据库后端\n");
		exit(1);
	}

	fclose(cfile);
	return 0;
}

/*
 *
 */
static int download_node_file(const char * url,const char * file, int reason)
{
	size_t ret;
	char	line[1024];
	if (reason)
		snprintf(line,sizeof(line),"curl '%s' 2>/dev/null",url);
	else
		snprintf(line,sizeof(line),"curl -z '%s' '%s' 2>/dev/null",file,url);
	FILE * fn = popen(line,"r");
	ret = fread(line,sizeof(char),sizeof(line)/sizeof(char),fn);
	if (ret)
	{
		FILE *f;
		if (reason) // broken
			f= fopen(file,"w");
		else
		{
			char * tmpfile = strdup(file);
			strcat(tmpfile,".tmp");
			f= fopen(tmpfile,"w");
			if (!f)
			{
				pclose (fn);
				return -1;
			}
			free(tmpfile);
		}

		ret = fwrite(line,sizeof(char),ret,f);
		while (ret)
		{
			ret = fread(line,sizeof(char),sizeof(line)/sizeof(char),fn);
			ret = fwrite(line,sizeof(char),ret,f);
		}
		fclose(f);
		if (!reason)
		{
			char * tmpfile = strdup(file);
			strcat(tmpfile,".tmp");
			rename(tmpfile,file);
			free(tmpfile);
		}
		reason = 0;
	}
	pclose(fn);
	return reason;
}

/*
 * scan nodes from line，push to a list
 */
static int nodesscanf( const char* line , void * ptr)
{
	int		s1,s2,s3,s4 ,port;

	NODE * node_ptr = g_new(NODE,1);

	switch(sscanf(line, "%d.%d.%d.%d:%d", &s1, &s2, &s3, &s4, &port))
	{
	case 4:
		port = DISTDB_DEFAULT_PORT;
	case 5:
		node_ptr->peer.sin_family = AF_INET;
		node_ptr->peer.sin_port = port;
		node_ptr->peer.sin_addr.s_addr = MAKEINET(s1,s2,s3,s4);
		ZEROWITHSIZE(node_ptr->peer.sin_zero);
		node_fromfile = g_list_append(node_fromfile,node_ptr);
		break;
	default:
		g_free(node_ptr);
		return -1;
	}
	return 0;
}


/*
 * seek to a chunk {}
 */
static int seek_chunk(FILE*file, const char * block)
{
	char	tmplate[1024];
	size_t	n = 1024;
	char*	linebuf = malloc(n);
	char*	p;

	size_t bytesread;

	while (!feof(file) && (bytesread=getline(&linebuf, &n, file)) != -1)
	{
		p = linebuf;
		while (*p == ' ' || *p == '\t' || *p=='\n')
			++p;

		if(*p=='#') continue;

		if(strncmp(p, block, MIN(strlen(block),n- (p - linebuf))))
			continue;
		fseek(file, strlen(block) -bytesread , SEEK_CUR);
		break;
	}
	free(linebuf);
	return 0;
}

/*
 * return 1 if broken file
 */
int check_node_file(const char * szfile)
{
	FILE * file = fopen(szfile,"r");

	if (!file)
		return 1; // cannot open ? broken!

	// TODO : check the format


	return 0;
}

/*
 * chunk_read
 */
static int chunk_read(FILE* file,int (*_readline)(const char * line,void * userptr ), const char * userptr )
{
	size_t n = 4096;
	int ret;
	char * linebuf = malloc(4096);
	char *p;
	int found=0;

	while (memset(linebuf, 0, 4096) && !feof(file) && getline(&linebuf, &n, file) != -1)
	{
		p = linebuf;
		while (*p == ' ' || *p == '\t' || *p == '\n')
			++p;
		if (*p && *p!='#')
		{
			if(found == 0)
			{
				// seek "{"
				if (*p != '{')
				{
					ret = -1;
					break;
				}else
				{
					found = 1;
				}
			}
			else if (found == 1 && *p == '}')
			{
				ret = 0;
				break;
			}
			else
			{
				if((ret = _readline(p,(void*)userptr)))
					break;
			}
		}
	}
	free(linebuf);
	return ret;

}

static int sscan_maps(const char * line , void * ptr)
{
	char * cuptr = * (char**) ptr;
	int id;
	char * name = cuptr;
	if(sscanf(line,"%[^ \t]%d",name,&id)!=2)
		return -1;
	cuptr += strlen(name) + 1;
	* (char**) ptr = cuptr ;

	groupmap--;
	groupmap->id = id;
	groupmap->name = name;
	groupcount++;
	return 0;
}

/*
 * read nodes into memory
 */
int read_nodes(const char * nodes_file)
{
	static char*	  groupmap_back_store;

	int ret;
	size_t n = 1024;
	char * linebuf;
	char * p;
	int found;

	char * cuptr ;

		// open the file
	FILE * nf = fopen(nodes_file, "r");
	if (!nf)
		return -1;
	linebuf = malloc(n);

	//read the group id to name map
	seek_chunk(nf, "groupmap");
	//

	groupmap_back_store = malloc(4096);
	cuptr = groupmap_back_store;
	groupmap = (typeof(groupmap)) (groupmap_back_store+4096);

	chunk_read(nf,sscan_maps,(void*)&cuptr);
				// groupid to name map;
				//char * p = groupmap_back_store;
	// for every groupname, read the groups
	int i;

	for (i = 0; i < groupcount; ++i)
	{
		char	chunk[500];
		int		id = groupmap[i].id ;
		fseek(nf,0,0);

		snprintf(chunk,499,"group %s",groupmap[i].name);

		seek_chunk(nf,chunk);
		chunk_read(nf,nodesscanf,0);
	}

	free(linebuf);
	return ret;
}

int main(int argc,char*argv[],char*env[])
{
	node_fromfile = g_list_alloc();

	getconfig(&cfg_progs, argc, argv);

	if (download_node_file(cfg_progs.url_nodefile, cfg_progs.startup_node_file,
	                       check_node_file(cfg_progs.startup_node_file)))
	{
		printf("unable to recover from the nodes.db crash\n");
		exit(EXIT_FAILURE);
	}


	distdb_initalize();


	//Turn to server mode
	distdb_enable_server(&distdbinfo,0);

	// read nodes
	if (read_nodes(cfg_progs.startup_node_file))
	{
		printf("unable to read %s\n",cfg_progs.startup_node_file);
		exit(EXIT_FAILURE);
	}
#ifndef DEBUG
	daemon(0,0);
#endif
	//start to connect to nodes at idle time
	while(sleep(rand()%20))
	{
		//connect to nodes, haha



	}

}
