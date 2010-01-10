/**
 * distdb.java - RPC binding for java
 * 
 */


public class distdb {
	static
	{
		System.loadLibrary("distdb.so");
	}
	public native static double test(double p);
	
    public static void main(String[] args)
    {
            double j;
            j = distdb.test(4);
            System.out.print(j);
    }
}
