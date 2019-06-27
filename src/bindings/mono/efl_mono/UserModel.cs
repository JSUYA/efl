using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Linq;
using System.ComponentModel;
using System.Reflection;

namespace Efl {

internal class ModelHelper
{
    /// FIXME Move this to eina_value.cs and be a static method of Value?
    static internal Eina.Value ValueFromProperty<T>(T source, PropertyInfo property)
    {
        return new Eina.Value(property.GetValue(source));
    }

    static internal void SetPropertyFromValue<T>(T target, PropertyInfo property, Eina.Value v)
    {
        property.SetValue(target, v.Unwrap());
    }

    /// <summary>Sets the properties of the <paramref name="child"/> from the properties of the given object
    /// <paramref name="o"/>.</summary>
    static internal void SetProperties<T>(T o, Efl.IModel child)
    {
        var properties = typeof(T).GetProperties();
        foreach(var prop in properties)
        {
            child.SetProperty(prop.Name, ValueFromProperty(o, prop));
        }
    }

    /// <summary>Sets the properties of <paramref name="o"/> from the properties of <paramref name="child"/>.</summary>
    static internal void GetProperties<T>(T o, Efl.IModel child)
    {
        var properties = typeof(T).GetProperties();
        foreach(var prop in properties)
        {
            using (var v = child.GetProperty(prop.Name))
            {
                SetPropertyFromValue(o, prop, v);
            }
        }
    }
}

public class UserModel<T> : Efl.MonoModelInternal, IDisposable
{
   public UserModel (Efl.Object parent = null) : base(Efl.MonoModelInternal.efl_mono_model_internal_class_get(), typeof(MonoModelInternal), parent, true)
   {
     var properties = typeof(T).GetProperties();
     foreach(var prop in properties)
     {
        AddProperty(prop.Name, System.IntPtr.Zero);
     }

     FinishInstantiation();
   }

   ~UserModel()
   {
       Dispose(false);
   }
   public void Add (T o)
   {
       Efl.IModel child = (Efl.IModel) this.AddChild();
       ModelHelper.SetProperties(o, child);
   }
}

}
