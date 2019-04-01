#pragma once

#include <ospray/ospray.h>

#include "../Backend.h"

#include <stdlib.h>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace RTW
{

  OSPFrameBufferFormat convert(RTWFrameBufferFormat format)
  {
    switch (format)
    {
    case RTW_FB_RGBA8:
      return OSP_FB_RGBA8;
    default:
      return OSP_FB_NONE;
    }
  }

  OSPTextureFormat convert(RTWTextureFormat format)
  {
    switch (format)
    {
    case RTW_TEXTURE_RGBA8:
      return OSP_TEXTURE_RGBA8;
    case RTW_TEXTURE_RGBA32F:
      return OSP_TEXTURE_RGBA32F;
    case RTW_TEXTURE_RGB8:
      return OSP_TEXTURE_RGB8;
    case RTW_TEXTURE_RGB32F:
      return OSP_TEXTURE_RGB32F;
    case RTW_TEXTURE_R8:
      return OSP_TEXTURE_R8;
    case RTW_TEXTURE_R32F:
      return OSP_TEXTURE_R32F;
    }
  }




  /*
   * Simple pass-through backend for OSPRay.
   */
  class OSPRayBackend : public Backend
  {
  public:
    RTWError Init(int *, const char **)
    {
      RTWError ret = RTW_UNKNOWN_ERROR;
      int ac = 1;
      const char* envArgs = getenv("VTKOSPRAY_ARGS");
      if (envArgs)
      {
        std::stringstream ss(envArgs);
        std::string arg;
        std::vector<std::string> args;
        while (ss >> arg)
        {
          args.push_back(arg);
        }
        ac = static_cast<int>(args.size() + 1);
        const char** av = new const char*[ac];
        av[0] = "pvOSPRay";
        for (int i = 1; i < ac; i++)
        {
          av[i] = args[i - 1].c_str();
        }
        try
        {
          ret = static_cast<RTWError>(ospInit(&ac, av));
        }
        catch (std::runtime_error &)
        {
          Shutdown();
        }
        delete[] av;
      }
      else
      {
        const char* av[] = { "pvOSPRay\0" };
        try
        {
          ret = static_cast<RTWError>(ospInit(&ac, av));
        }
        catch (std::runtime_error &)
        {
          Shutdown();
        }
      }
      return ret;
    }

    void Shutdown()
    {
      ospShutdown();
    }

    bool IsSupported(RTWFeature feature) const
    {
      switch (feature)
      {
      case RTW_DEPTH_NORMALIZATION:
        return false;
      case RTW_OPENGL_INTEROP:
        return false;
      case RTW_ANIMATED_PARAMETERIZATION:
        return false;
      case RTW_INSTANCING:
        return true;
      case RTW_DENOISER:
        return false; // OpenImageDenoise is an external lib outside of the backend
      case RTW_DEPTH_COMPOSITING:
        return true;
      }
      return false;
    }

    RTWData NewData(size_t numElements, RTWDataType dataType, const void *source, const uint32_t dataCreationFlags)
    {
      return reinterpret_cast<RTWData>(ospNewData(numElements, static_cast<OSPDataType>(dataType), source, dataCreationFlags));
    }

    RTWGeometry NewGeometry(const char *type)
    {
      return reinterpret_cast<RTWGeometry>(ospNewGeometry(type));
    }

    RTWTexture NewTexture(const char* type)
    {
      return reinterpret_cast<RTWTexture>(ospNewTexture(type));
    }

    RTWLight NewLight(RTWRenderer renderer, const char *type)
    {
      return reinterpret_cast<RTWLight>(ospNewLight(reinterpret_cast<OSPRenderer>(renderer), type));
    }

    RTWLight NewLight2(const char *renderer_type, const char *light_type)
    {
      return reinterpret_cast<RTWLight>(ospNewLight2(renderer_type, light_type));
    }

    RTWLight NewLight3(const char *light_type)
    {
      return reinterpret_cast<RTWLight>(ospNewLight3(light_type));
    }

    RTWMaterial NewMaterial(RTWRenderer renderer, const char *material_type)
    {
      return reinterpret_cast<RTWMaterial>(ospNewMaterial(reinterpret_cast<OSPRenderer>(renderer), material_type));
    }

    RTWMaterial NewMaterial2(const char *renderer_type, const char *material_type)
    {
      return reinterpret_cast<RTWMaterial>(ospNewMaterial2(renderer_type, material_type));
    }

    RTWVolume NewVolume(const char *type)
    {
      return reinterpret_cast<RTWVolume>(ospNewVolume(type));
    }

    RTWTransferFunction NewTransferFunction(const char *type)
    {
      return reinterpret_cast<RTWTransferFunction>(ospNewTransferFunction(type));
    }

    RTWRenderer NewRenderer(const char *type)
    {
      return reinterpret_cast<RTWRenderer>(ospNewRenderer(type));
    }

    RTWCamera NewCamera(const char *type)
    {
      return reinterpret_cast<RTWCamera>(ospNewCamera(type));
    }

    RTWModel NewModel()
    {
      return reinterpret_cast<RTWModel>(ospNewModel());
    }

    RTWGeometry NewInstance(RTWModel modelToInstantiate, const rtw::affine3f &transform)
    {
      osp::affine3f xfm;
      memcpy(&xfm, &transform, sizeof(osp::affine3f));
      return reinterpret_cast<RTWGeometry>(ospNewInstance(reinterpret_cast<OSPModel>(modelToInstantiate), xfm));
    }

    RTWFrameBuffer NewFrameBuffer(const rtw::vec2i &size, const RTWFrameBufferFormat format, const uint32_t frameBufferChannels)
    {
      return reinterpret_cast<RTWFrameBuffer>(ospNewFrameBuffer(osp::vec2i{ size.x, size.y }, convert(format), frameBufferChannels));
    }

    void Release(RTWObject object)
    {
      ospRelease(reinterpret_cast<OSPObject>(object));
    }

    void AddGeometry(RTWModel model, RTWGeometry geometry)
    {
      ospAddGeometry(reinterpret_cast<OSPModel>(model), reinterpret_cast<OSPGeometry>(geometry));
    }

    void AddVolume(RTWModel model, RTWVolume volume)
    {
      ospAddVolume(reinterpret_cast<OSPModel>(model), reinterpret_cast<OSPVolume>(volume));
    }

    void SetString(RTWObject object, const char *id, const char *s)
    {
      ospSetString(reinterpret_cast<OSPObject>(object), id, s);
    }

    void SetObject(RTWObject object, const char *id, RTWObject other)
    {
      ospSetObject(reinterpret_cast<OSPObject>(object), id, reinterpret_cast<OSPObject>(other));
    }

    void SetData(RTWObject object, const char *id, RTWData data)
    {
      ospSetData(reinterpret_cast<OSPObject>(object), id, reinterpret_cast<OSPData>(data));
    }

    void SetMaterial(RTWGeometry geometry, RTWMaterial material)
    {
      ospSetMaterial(reinterpret_cast<OSPGeometry>(geometry), reinterpret_cast<OSPMaterial>(material));
    }

    void Set1i(RTWObject object, const char *id, int32_t x)
    {
      ospSet1i(reinterpret_cast<OSPObject>(object), id, x);
    }

    void Set1f(RTWObject object, const char *id, float x)
    {
      ospSet1f(reinterpret_cast<OSPObject>(object), id, x);
    }

    void Set2f(RTWObject object, const char *id, float x, float y)
    {
      ospSet2f(reinterpret_cast<OSPObject>(object), id, x, y);
    }

    void Set2i(RTWObject object, const char *id, int x, int y)
    {
      ospSet2i(reinterpret_cast<OSPObject>(object), id, x, y);
    }

    void Set3i(RTWObject object, const char *id, int x, int y, int z)
    {
      ospSet3i(reinterpret_cast<OSPObject>(object), id, x, y, z);
    }

    void Set3f(RTWObject object, const char *id, float x, float y, float z)
    {
      ospSet3f(reinterpret_cast<OSPObject>(object), id, x, y, z);
    }

    void Set4f(RTWObject object, const char *id, float x, float y, float z, float w)
    {
      ospSet4f(reinterpret_cast<OSPObject>(object), id, x, y, z, w);
    }

    RTWError SetRegion(RTWVolume volume, void *source, const rtw::vec3i &regionCoords, const rtw::vec3i &regionSize)
    {
      return static_cast<RTWError>(ospSetRegion(reinterpret_cast<OSPVolume>(volume), source,
        osp::vec3i{ regionCoords.x, regionCoords.y, regionCoords.z },
        osp::vec3i{ regionSize.x, regionSize.y, regionSize.z }));
    }

    void Commit(RTWObject object)
    {
      ospCommit(reinterpret_cast<OSPObject>(object));
    }

    float RenderFrame(RTWFrameBuffer frameBuffer, RTWRenderer renderer, const uint32_t frameBufferChannels)
    {
      return ospRenderFrame(reinterpret_cast<OSPFrameBuffer>(frameBuffer), reinterpret_cast<OSPRenderer>(renderer), frameBufferChannels);
    }

    void FrameBufferClear(RTWFrameBuffer frameBuffer, const uint32_t frameBufferChannels)
    {
      ospFrameBufferClear(reinterpret_cast<OSPFrameBuffer>(frameBuffer), frameBufferChannels);
    }

    const void* MapFrameBuffer(RTWFrameBuffer frameBuffer, const RTWFrameBufferChannel channel)
    {
      return ospMapFrameBuffer(reinterpret_cast<OSPFrameBuffer>(frameBuffer), static_cast<OSPFrameBufferChannel>(channel));
    }

    void UnmapFrameBuffer(const void *mapped, RTWFrameBuffer frameBuffer)
    {
      ospUnmapFrameBuffer(mapped, reinterpret_cast<OSPFrameBuffer>(frameBuffer));
    }

    void SetDepthNormalizationGL(RTWFrameBuffer /*frameBuffer*/, float /*clipMin*/, float /*clipMax*/)
    {
      // not supported
    }

    int GetColorTextureGL(RTWFrameBuffer /*frameBuffer*/)
    {
      // not supported
      return 0;
    }

    int GetDepthTextureGL(RTWFrameBuffer /*frameBuffer*/)
    {
      // not supported
      return 0;
    }
  };
}
