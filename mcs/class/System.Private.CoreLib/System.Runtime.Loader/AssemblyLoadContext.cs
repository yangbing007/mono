using System.IO;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Threading;

namespace System.Runtime.Loader
{
	partial class AssemblyLoadContext
	{

		[MethodImplAttribute (MethodImplOptions.InternalCall)]
		extern static IntPtr InitializeAssemblyLoadContext (IntPtr gcHandleAssemblyLoadContext, bool representsTPALoadContext, bool isCollectible);

		static void PrepareForAssemblyLoadContextRelease (IntPtr nativeAssemblyLoadContext, IntPtr assemblyLoadContextStrong)
		{
		}

		static IntPtr InternalLoadUnmanagedDllFromPath (string unmanagedDllPath)
		{
			throw new NotImplementedException ();
		}

		[System.Security.DynamicSecurityMethod] // Methods containing StackCrawlMark local var has to be marked non-inlineable        
		Assembly InternalLoadFromPath (string assemblyPath, string nativeImagePath)
		{
			assemblyPath = assemblyPath.Replace ('\\', Path.DirectorySeparatorChar);
			// TODO: Handle nativeImagePath
			return InternalLoadFile (assemblyPath, _nativeAssemblyLoadContext);
		}

		internal Assembly InternalLoad (byte[] arrAssembly, byte[] arrSymbols)
		{
			throw new NotImplementedException ();
		}

		public static Assembly[] GetLoadedAssemblies ()
		{
			throw new NotImplementedException ();
		}

		public static AssemblyLoadContext GetLoadContext (Assembly assembly)
		{
			throw new NotImplementedException ();
		}

		public void SetProfileOptimizationRoot (string directoryPath)
		{
		}

		public void StartProfileOptimization (string profile)
		{
		}

		[MethodImplAttribute (MethodImplOptions.InternalCall)]
		extern static Assembly InternalLoadFile (string assemblyFile, IntPtr nativeALC);

		internal static Assembly DoAssemblyResolve (string name)
		{
			return AssemblyResolve (null, new ResolveEventArgs (name));
		}
	}
}
