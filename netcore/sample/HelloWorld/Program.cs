using System;
using System.Reflection;

namespace HelloWorld
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("Hello World!");

            //var str = "System.Net.Http, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a";
	    var str = "System.Collections.NonGeneric";
#if false
            var a = typeof(ByteArrayContent);
            Console.WriteLine(a.Assembly.FullName);
#endif
            var aname = new AssemblyName(str);
            var asm1 = Assembly.Load(aname);
            var p = asm1.Location;
            Console.WriteLine("{0}", p);

            var asm2 = Assembly.LoadFile(p);

	    Console.WriteLine ("asm1 = <<{0}>> : {1}", asm1.FullName, asm1.GetType().FullName);
	    Console.WriteLine ("asm2 = <<{0}>> : {1}", asm2.FullName, asm2.GetType().FullName);

	    /* on .NET Framework this prints True, True */
            Console.WriteLine("are they equal? {0}", asm1 == asm2);
            Console.WriteLine("are they reference equal? {0}", Object.ReferenceEquals(asm1, asm2));

            Console.WriteLine(typeof(object).Assembly.FullName);
            Console.WriteLine(System.Reflection.Assembly.GetEntryAssembly());
            return;

        }
    }
}

#if false
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
	    var asm1 = ctx.LoadFromAssemblyName  (aname);
	    var p = asm1.Location;
	    Console.WriteLine ("{0}", p);

	    var asm2 = Assembly.LoadFile (p); /* does this work on .NET Framework?  Is Mono just bonkers here? */

	    Console.WriteLine ("are they equal? {0}", asm1 == asm2);
	    Console.WriteLine ("are they reference equal? {0}", Object.ReferenceEquals (asm1, asm2));

            Console.WriteLine(typeof(object).Assembly.FullName);
            Console.WriteLine(System.Reflection.Assembly.GetEntryAssembly ());
        }
    }
}
#endif
