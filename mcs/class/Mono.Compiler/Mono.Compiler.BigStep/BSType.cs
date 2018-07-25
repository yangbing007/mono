using System;
using System.Reflection;

using LLVMSharp;

namespace Mono.Compiler.BigStep {

	/// <summary>
	///   Tie together runtime types and LLVM types.
	/// </summary>
	public class BSType
	{
		// FIXME: is this the most useful one?
		RuntimeTypeHandle rttype;

		private BSType (TypeInfo t) {
			rttype = t.TypeHandle;
		}

		public static BSType FromTypeInfo (TypeInfo t) {
			// TODO: cache?
			return new BSType (t);
		}



		public LLVMTypeRef Lowered {
			get {
				throw new NotImplementedException ("BSType.Lowered");
			}
		}
	}
}
