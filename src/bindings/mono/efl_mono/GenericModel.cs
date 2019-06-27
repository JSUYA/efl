using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Linq;
using System.ComponentModel;
using Eina;

namespace Efl {

public class GenericModel<T> : Efl.Object, Efl.IModel, IDisposable
{
   private Efl.IModel model;

   public GenericModel (Efl.IModel model, Efl.Object parent = null) : base(parent)
   {
       this.model = model;
   }

   public Eina.Iterator< System.String> Properties {
      get { return GetProperties(); }
   }
   public  uint ChildrenCount {
      get { return GetChildrenCount(); }
   }

   public Eina.Iterator<System.String> GetProperties()
   {
       return model.GetProperties();
   }
   public Eina.Value GetProperty(  System.String property)
   {
       return model.GetProperty(property);
   }
   public Eina.Future SetProperty(  System.String property,   Eina.Value value)
   {
       return model.SetProperty(property, value);
   }
   public uint GetChildrenCount()
   {
       return model.GetChildrenCount();
   }
   public Eina.Future GetPropertyReady(  System.String property)
   {
       return model.GetPropertyReady(property);
   }
   public Eina.Future GetChildrenSlice(  uint start,   uint count)
   {
       return model.GetChildrenSlice(start, count);
   }
   public void Add(T o)
   {
      Efl.IModel child = (Efl.IModel)this.AddChild();
      ModelHelper.SetProperties(o, child);
   }
   public Efl.Object AddChild()
   {
       return model.AddChild();
   }
   public void DelChild( Efl.Object child)
   {
       model.DelChild(child);
   }
   async public System.Threading.Tasks.Task<T> GetAtAsync(uint index)
   {
       using (Eina.Value v = await GetChildrenSliceAsync(index, 1))
       {
           if (v.GetValueType().IsContainer())
           {
               var child = (Efl.IModel)v[0];
               T obj = (T)Activator.CreateInstance(typeof(T), new System.Object[] {});
               ModelHelper.GetProperties(obj, child);
               return obj;
           }
           else
           {
               throw new System.InvalidOperationException("GetChildrenSlice must have returned a container");
           }
       }
   }
   public System.Threading.Tasks.Task<Eina.Value> SetPropertyAsync(  System.String property,  Eina.Value value, System.Threading.CancellationToken token=default(System.Threading.CancellationToken))
   {
       return model.SetPropertyAsync(property, value, token);
   }
   public System.Threading.Tasks.Task<Eina.Value> GetPropertyReadyAsync(  System.String property, System.Threading.CancellationToken token=default(System.Threading.CancellationToken))
   {
       return model.GetPropertyReadyAsync(property, token);
   }
   public System.Threading.Tasks.Task<Eina.Value> GetChildrenSliceAsync(  uint start,  uint count, System.Threading.CancellationToken token=default(System.Threading.CancellationToken))
   {
       return model.GetChildrenSliceAsync(start, count, token);
   }
   public event EventHandler<Efl.IModelPropertiesChangedEvt_Args> PropertiesChangedEvt
   {
      add {
          model.PropertiesChangedEvt += value;
      }
      remove {
          model.PropertiesChangedEvt -= value;
      }
   }
   public event EventHandler<Efl.IModelChildAddedEvt_Args> ChildAddedEvt
   {
      add {
          model.ChildAddedEvt += value;
      }
      remove {
          model.ChildAddedEvt -= value;
      }
   }
   public event EventHandler<Efl.IModelChildRemovedEvt_Args> ChildRemovedEvt
   {
      add {
          model.ChildRemovedEvt += value;
      }
      remove {
          model.ChildRemovedEvt -= value;
      }
   }
   public event EventHandler ChildrenCountChangedEvt
   {
      add {
          model.ChildrenCountChangedEvt += value;
      }
      remove {
          model.ChildrenCountChangedEvt -= value;
      }
   }
}

}
