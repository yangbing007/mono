using System;
using NUnit.Framework;
using NUnit.Framework.Constraints;
using Mono.Compiler;

using SimpleJit.CIL;

namespace MonoTests.Mono.CompilerInterface
{
	public abstract partial class MiniRegressionTestsBasic : MiniRegressionTests
	{
		public override Type GetRegressionSuite () {
			return typeof (MiniRegressionTestsBasic);
		}

	}

	[TestFixture]
	public class MiniRegressionTestsBasicManagedJIT : MiniRegressionTestsBasic
	{
		[Test, TestCaseSource (typeof (MiniRegressionTestsBasicManagedJIT), "Regressions")]
		public void TestRegression (System.Reflection.MethodInfo methodInfo) {
			ExecuteRegressionTest (methodInfo);
		}

		public override ICompiler GetCompiler () {
			return new ManagedJIT ();
		}
	}

	[TestFixture]
	public class MiniRegressionTestsBasicMiniCompiler : MiniRegressionTestsBasic
	{
		[Test, TestCaseSource (typeof (MiniRegressionTestsBasicMiniCompiler), "Regressions")]
		public void TestRegression (System.Reflection.MethodInfo methodInfo) {
			ExecuteRegressionTest (methodInfo);
		}

		public override ICompiler GetCompiler () {
			return new MiniCompiler ();
		}
	}
}
