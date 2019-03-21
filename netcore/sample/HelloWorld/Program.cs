using System;
using System.Reflection;
using System.Runtime.Loader;

namespace HelloWorld
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("Hello World!");

	    var ctx = AssemblyLoadContext.Default;

	    var aname = new AssemblyName ("System.Collections.NonGeneric");
#if false
	    Assembly.Load (aname); /* this still has null as the load */
#else
	    ctx.LoadFromAssemblyName  (aname);
#endif

            Console.WriteLine(typeof(object).Assembly.FullName);
            Console.WriteLine(System.Reflection.Assembly.GetEntryAssembly ());
        }
    }
}
