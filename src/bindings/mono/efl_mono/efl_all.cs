using System;
using System.Runtime.InteropServices;
using System.Threading;

using static efl.UnsafeNativeMethods;

namespace efl {

static class UnsafeNativeMethods {
    [DllImport(efl.Libs.Ecore)] public static extern void ecore_init();
    [DllImport(efl.Libs.Ecore)] public static extern void ecore_shutdown();
    [DllImport(efl.Libs.Evas)] public static extern void evas_init();
    [DllImport(efl.Libs.Evas)] public static extern void evas_shutdown();
    [DllImport(efl.Libs.Elementary)] public static extern int elm_init(int argc, IntPtr argv);
    [DllImport(efl.Libs.Elementary)] public static extern void elm_policy_set(int policy, int policy_detail);
    [DllImport(efl.Libs.Elementary)] public static extern void elm_shutdown();
    [DllImport(efl.Libs.Elementary)] public static extern void elm_run();
    [DllImport(efl.Libs.Elementary)] public static extern void elm_exit();
}

public enum Components {
    Basic,
    Ui
}

public static class All {
    private static bool InitializedUi = false;

    public static void Init(efl.Components components=Components.Basic) {
        eina.Config.Init();
        efl.eo.Config.Init();
        ecore_init();
        evas_init();
        eldbus.Config.Init();

        if (components == Components.Ui) {
            efl.ui.Config.Init();
            InitializedUi = true;
        }
    }

    /// <summary>Shutdowns all EFL subsystems.</summary>
    public static void Shutdown() {
        // Try to cleanup everything before actually shutting down.
        System.GC.Collect();
        System.GC.WaitForPendingFinalizers();

        if (InitializedUi)
            efl.ui.Config.Shutdown();
        eldbus.Config.Shutdown();
        evas_shutdown();
        ecore_shutdown();
        efl.eo.Config.Shutdown();
        eina.Config.Shutdown();
    }
}

// Placeholder. Will move to elm_config.cs later
namespace ui {

public static class Config {
    public static void Init() {
        // TODO Support elm command line arguments
#if WIN32 // Not a native define, we define it in our build system
        // Ecore_Win32 uses OleInitialize, which requires single thread apartments
        if (Thread.CurrentThread.GetApartmentState() != ApartmentState.STA)
            throw new InvalidOperationException("UI Applications require STAThreadAttribute in Main()");
#endif
        elm_init(0, IntPtr.Zero);

        elm_policy_set((int)elm.Policy.Quit, (int)elm.policy.Quit.Last_window_hidden);
    }
    public static void Shutdown() {
        elm_shutdown();
    }

    public static void Run() {
        elm_run();
    }

    public static void Exit() {
        elm_exit();
    }
}

}

}
