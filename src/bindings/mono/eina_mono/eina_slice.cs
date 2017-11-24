using System;
using System.Runtime.InteropServices;

namespace eina {

public interface ISliceBase
{
    UIntPtr Len {get;set;}
    IntPtr Mem {get;set;}

    int Length {get;set;}
};

[StructLayout(LayoutKind.Sequential)]
public struct Slice : ISliceBase
{
    public UIntPtr Len {get;set;}
    public IntPtr Mem {get;set;}

    public int Length
    {
        get { return (int) Len; }
        set { Len = (UIntPtr) value; }
    }

    public Slice(IntPtr mem, UIntPtr len)
    {
        Mem = mem;
        Len = len;
    }

    public Slice PinnedDataSet(IntPtr mem, UIntPtr len)
    {
        Mem = mem;
        Len = len;
        return this;
    }
}

[StructLayout(LayoutKind.Sequential)]
public struct Rw_Slice : ISliceBase
{
    public UIntPtr Len {get;set;}
    public IntPtr Mem {get;set;}

    public int Length
    {
        get { return (int) Len; }
        set { Len = (UIntPtr) value; }
    }

    public Rw_Slice(IntPtr mem, UIntPtr len)
    {
        Mem = mem;
        Len = len;
    }

    public Rw_Slice PinnedDataSet(IntPtr mem, UIntPtr len)
    {
        Mem = mem;
        Len = len;
        return this;
    }

    Slice ToSlice()
    {
        var r = new Slice();
        r.Mem = Mem;
        r.Len = Len;
        return r;
    }
}

}

public static class Eina_SliceUtils
{
    public static byte[] GetBytes(this eina.ISliceBase slc)
    {
        var size = (int)(slc.Len);
        byte[] mArray = new byte[size];
        Marshal.Copy(slc.Mem, mArray, 0, size);
        return mArray;
    }
}
