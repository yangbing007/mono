using System.IO;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
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

		[MethodImplAttribute (MethodImplOptions.InternalCall)]
		private static extern IntPtr GetLoadContextForAssembly (RuntimeAssembly assembly);

#region copied from CoreCLR
		// Returns the load context in which the specified assembly has been loaded
		public static AssemblyLoadContext GetLoadContext(Assembly assembly)
		{
			if (assembly == null)
			{
				throw new ArgumentNullException(nameof(assembly));
			}

			AssemblyLoadContext loadContextForAssembly = null;

			RuntimeAssembly rtAsm = assembly as RuntimeAssembly;

			// We only support looking up load context for runtime assemblies.
			if (rtAsm != null)
			{
				IntPtr ptrAssemblyLoadContext = GetLoadContextForAssembly(rtAsm);
				if (ptrAssemblyLoadContext == IntPtr.Zero)
				{
					// If the load context is returned null, then the assembly was bound using the TPA binder
					// and we shall return reference to the active "Default" binder - which could be the TPA binder
					// or an overridden CLRPrivBinderAssemblyLoadContext instance.
					loadContextForAssembly = AssemblyLoadContext.Default;
				}
				else
				{
					loadContextForAssembly = (AssemblyLoadContext)(GCHandle.FromIntPtr(ptrAssemblyLoadContext).Target);
				}
			}

			return loadContextForAssembly;
		}
#endregion

		public void SetProfileOptimizationRoot (string directoryPath)
		{
		}

		public void StartProfileOptimization (string profile)
		{
		}

		[MethodImplAttribute (MethodImplOptions.InternalCall)]
		extern static Assembly InternalLoadFile (string assemblyFile, IntPtr nativeALC);

		// XXX Aleksey - added by vargaz to do AssembyName stuff.
		// Probably should just call OnAssemblyResolve.  Especially
		// since ALC splices multiple event handlers on here.
		internal static Assembly DoAssemblyResolve (string name)
		{
			return AssemblyResolve (null, new ResolveEventArgs (name));
		}

		// XXX Aleksey - This is from https://github.com/dotnet/coreclr/blob/ea10aaccb09fe30f0444b821fd8d90d9257dd402/src/System.Private.CoreLib/src/System/Runtime/Loader/AssemblyLoadContext.CoreCLR.cs
		// The runtime should invoke this (DoAssemblyResolve - for us)
#if false
		// This method is called by the VM.
        private static RuntimeAssembly OnAssemblyResolve(RuntimeAssembly assembly, string assemblyFullName)
        {
            return InvokeResolveEvent(AssemblyResolve, assembly, assemblyFullName);
        }

        private static RuntimeAssembly InvokeResolveEvent(ResolveEventHandler eventHandler, RuntimeAssembly assembly, string name)
        {
            if (eventHandler == null)
                return null;

            var args = new ResolveEventArgs(name, assembly);

            foreach (ResolveEventHandler handler in eventHandler.GetInvocationList())
            {
                Assembly asm = handler(null /* AppDomain */, args);
                RuntimeAssembly ret = GetRuntimeAssembly(asm);
                if (ret != null)
                    return ret;
            }

            return null;
        }

        private static RuntimeAssembly GetRuntimeAssembly(Assembly asm)
        {
            return
                asm == null ? null :
                asm is RuntimeAssembly rtAssembly ? rtAssembly :
                asm is System.Reflection.Emit.AssemblyBuilder ab ? ab.InternalAssembly :
                null;
        }
    }
#endif	
	}
}
