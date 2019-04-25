// Compiler options: -r:test-multi-netmodule-2-dll1.dll

using System;
using System.Reflection;

public class M4 {
	public static int Main () {
		M2 m2 = new M2();

		// load the same netmodule on behalf of annother assembly
		var DLL = Assembly.LoadFile(System.IO.Path.GetFullPath ("test-multi-netmodule-3-dll2.dll"));
	        var m3Type = DLL.GetType("M3");
	        var m3 = Activator.CreateInstance(m3Type);
	        var m3m1Field = m3Type.GetField("m1");

		var m3asm = m3Type.Assembly;
		var m3m1asm = m3m1Field.DeclaringType.Assembly;
    		Console.WriteLine("M3    assembly:" + m3asm);
		Console.WriteLine("M3.M1 assembly:" + m3m1asm);

		var m2asm = typeof (M2).Assembly;
		var m2m1asm = m2.m1.GetType().Assembly;
		Console.WriteLine("M2    assembly:" + m2asm);
		Console.WriteLine("M2.M1 assembly:" + m2m1asm);

		bool fail = false;
		if (m3asm != m3m1asm) {
			Console.WriteLine ("M3 and M3.M1 in different assemblies");
			fail = true;
		}
		if (m2asm != m2m1asm) {
			Console.WriteLine ("M2 and M2.M1 in different assemblies");
			fail = true;
		}

		if (m3m1asm == m2m1asm) {
			Console.WriteLine ("M3.M1 and M2.M1 in the same assembly");
			fail = true;
		}

		return fail ? 1 : 0;
	}
}
