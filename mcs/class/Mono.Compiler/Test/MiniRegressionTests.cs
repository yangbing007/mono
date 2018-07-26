using System;
using System.Collections;
using NUnit.Framework;
using Mono.Compiler;

using SimpleJit.CIL;

namespace MonoTests.Mono.CompilerInterface
{
	public abstract class MiniRegressionTests {
		public abstract ICompiler GetCompiler ();

		public abstract Type GetRegressionSuite ();

		public IRuntimeInformation RuntimeInfo = new RuntimeInformation ();

		internal static int ParseExpected (string name) {
			Assert.IsTrue (name.StartsWith ("test_"));
			int j;
			for (j = 5; j < name.Length; ++j)
				if (!Char.IsDigit (name [j]))
					break;
			return Int32.Parse (name.Substring (5, j - 5));
		}

		public int RunGuest (MethodInfo mcmi) {
			NativeCodeHandle nativeCode;
			var result = GetCompiler ().CompileMethod (RuntimeInfo, mcmi, CompilationFlags.None, out nativeCode);
			InstalledRuntimeCode irc = RuntimeInfo.InstallCompilationResult (result, mcmi, nativeCode);
			return (int) RuntimeInfo.ExecuteInstalledMethod (irc);
		}

		public void ExecuteRegressionTest (System.Reflection.MethodInfo srmi) {
			MethodInfo mcmi = GetCompilerMethodInfo (srmi);

			int expected_result = ParseExpected (srmi.Name);
			int guest_result = RunGuest (mcmi);
			int host_result = (int) srmi.Invoke (null, null);

			Assert.AreEqual (host_result, guest_result, "expected by test: " + expected_result);
		}

		public MethodInfo GetCompilerMethodInfo (System.Reflection.MethodInfo srmi) {
				ClassInfo ci = ClassInfo.FromType (GetRegressionSuite ());
				/* FIXME: get MethodInfo somehow directly via SR.MethodInfo? */
				return RuntimeInfo.GetMethodInfoFor (ci, srmi.Name);
		}

		public IEnumerable Regressions {
			get {
				Type t = GetRegressionSuite ();
				string not_compiler = "!" + GetCompiler ().Name;

				foreach (System.Reflection.MethodInfo method in t.GetMethods ()) {
					if (!method.Name.StartsWith ("test_"))
						continue;

					/* check for disabled tests */
					var attrs = method.GetCustomAttributes (typeof (CategoryAttribute), false);
					bool skip = false;
					foreach (CategoryAttribute attr in attrs) {
						if (attr.Category == not_compiler) {
							skip = true;
							break;
						}
					}
					if (skip)
						continue;

					yield return method;
				}
			}
		}
	}


	[AttributeUsageAttribute(AttributeTargets.All, Inherited = true, AllowMultiple = true)]
	public class CategoryAttribute : Attribute {
		public CategoryAttribute (string category) {
			Category = category;
		}

		public string Category {
			get; set;
		}
	}
}
