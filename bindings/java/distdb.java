/**
 * distdb.java - RPC binding for java
 * 
 */
import java.*;
public class distdb {
	static
	{
		System.loadLibrary("libdistdb");
	}
	public native static double testd(double p);
	
    public static void main(String[] args)
    {
    		double j;
            j = distdb.testd(4);
            System.out.print(j);
    }
}
