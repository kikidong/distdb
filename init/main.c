
/* main - main file
 *
 * Copyright (C) 2009-2010 microcai
 *
 * This software is Public Domain
 *
 * For more infomation see COPYING file shipped with this software.
 *
 * If you have any question with this software, please contract microcai, the
 * original writer of this software.
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

#include "../include/inifile.h"
#include "prase.h"

#include "../include/nodes.h"
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

	/*
	 *
	 */

}cfg_progs;

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


int main(int argc,char*argv[],char*env[])
{
	getconfig(&cfg_progs, argc, argv);

	if (download_node_file(cfg_progs.url_nodefile, cfg_progs.startup_node_file,
	                       check_node_file(cfg_progs.startup_node_file)))
	{
		printf("unable to recover from the nodes.db crash\n");
		exit(EXIT_FAILURE);
	}

	// read nodes
	if (read_nodes(cfg_progs.startup_node_file))
	{
		printf("unable to read %s\n",cfg_progs.startup_node_file);
		exit(EXIT_FAILURE);
	}
	distdb_initalize();

	distdb_enable_server(0,0);

	distdb_initalize();

	distdb_enable_server(0,0);

	//listen on local address
	open_nodes_socket();

	//start to connect to nodes at idle time
	start_connect_nodes();

	//the big event loop (main program defined in ../src)
//	return event_loop();
}
