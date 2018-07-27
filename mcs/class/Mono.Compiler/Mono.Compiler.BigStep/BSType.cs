using System;
using System.Reflection;

using SimpleJit.Metadata;
using Mono.Compiler;

using LLVMSharp;

namespace Mono.Compiler.BigStep {

	/// <summary>
	///   Tie together runtime types and LLVM types.
	/// </summary>
	public class BSType
	{
		ClrType rttype;
		LLVMTypeRef? lowered;

		private BSType (ClrType t)
		{
			rttype = t;
		}

		private BSType (BSType t, LLVMTypeRef l)
			: this (t.rttype)
		{
			lowered = l;
		}

		public static BSType FromClrType (ClrType t)
		{
			// TODO: cache?
			return new BSType (t);
		}

		public static BSType FromTypeInfo (TypeInfo t)
		{
			return FromClrType (RuntimeInformation.ClrTypeFromType (t));
		}

		public LLVMTypeRef Lowered {
			get {
				if (lowered != null)
					return (LLVMTypeRef)lowered;
				else
					throw new NotImplementedException ("Don't know how to lower " + rttype.ToString ());
			}
		}

		internal BSType LowerAs (LLVMTypeRef l)
		{
			return new BSType (this, l);
		}


	}

	/// <summary>
	///   Some predefined BSType values from the runtime
	/// </summary>
	struct BSTypes {
		public readonly BSType VoidType;
		public readonly BSType Int32Type;
		public readonly BSType Int64Type;

		internal BSTypes (IRuntimeInformation runtimeInfo) {
			VoidType = BSType.FromClrType (runtimeInfo.VoidType).LowerAs (LLVMTypeRef.VoidType ());
			Int32Type = BSType.FromClrType (runtimeInfo.Int32Type).LowerAs (LLVMTypeRef.Int32Type ());
			Int64Type = BSType.FromClrType (runtimeInfo.Int64Type).LowerAs (LLVMTypeRef.Int64Type ());
		}
	}

}
