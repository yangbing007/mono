using System;
using System.Collections.Generic;
using LLVMSharp;


namespace Mono.Compiler.BigStep
{

	public struct ArgStackValue {
		public LLVMValueRef Ptr;
		public BSType StoredType;
	}


	/// <summary>
	///   The argument stack.
	///
	///   We are going to want to clone this thing
	/// </summary>
	class ArgStack
	{
		//WISH: this really ought to be an immutable collection
		private Stack<ArgStackValue> data;

		public ArgStack () : this (new Stack<ArgStackValue> ()) {}

		ArgStack (Stack<ArgStackValue> data) {
			this.data = data;
		}

		public ArgStack Clone () {
			ArgStackValue[] arr = new ArgStackValue[data.Count];
			data.CopyTo (arr, 0);
			Array.Reverse (arr);
			return new ArgStack (new Stack<ArgStackValue>(arr));
		}

		public void Push (ArgStackValue x)
		{
			data.Push (x);
		}

		public ArgStackValue Pop ()
		{
			return data.Pop ();
		}

		public int Count
		{
			get { return data.Count; }
		}
	}

}
