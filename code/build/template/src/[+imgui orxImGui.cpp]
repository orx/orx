//! Includes

#include "orx.h"
#include "imgui.h"


//! Prototypes

orxSTATUS   orxFASTCALL orxImGui_Init();
void        orxFASTCALL orxImGui_Exit();


//! Code

#ifndef orxIMGUI_HEADER_ONLY

static orxBOOL sbInFrame = orxFALSE;

static void *orxImGui_Allocate(size_t _sSize, void *)
{
  return orxMemory_Allocate((orxU32)_sSize, orxMEMORY_TYPE_TEMP);
}

static void orxImGui_Free(void *_pData, void *)
{
  orxMemory_Free(_pData);
}

static const char *orxImGui_GetClipboard(void *)
{
  return orxSystem_GetClipboard();
}

static void orxImGui_SetClipboard(void *, const char *_acText)
{
  orxSystem_SetClipboard(_acText);
}

static orxSTATUS orxFASTCALL orxImGui_BeginFrame(const orxEVENT *_pstEvent)
{
  if(!sbInFrame)
  {
    sbInFrame = orxTRUE;

    ImGuiIO &rstIO = ImGui::GetIO();

    rstIO.DeltaTime = orxClock_GetInfo(orxClock_FindFirst(-1.0f, orxCLOCK_TYPE_CORE))->fDT;

    orxDisplay_GetScreenSize(&rstIO.DisplaySize.x, &rstIO.DisplaySize.y);
    orxVECTOR vFramebufferSize;
    orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);
    orxConfig_GetVector(orxDISPLAY_KZ_CONFIG_FRAMEBUFFER_SIZE, &vFramebufferSize);
    orxConfig_PopSection();
    rstIO.DisplayFramebufferScale.x = vFramebufferSize.fX / rstIO.DisplaySize.x;
    rstIO.DisplayFramebufferScale.y = vFramebufferSize.fY / rstIO.DisplaySize.y;

    orxVECTOR vMousePos;
    orxMouse_GetPosition(&vMousePos);
    rstIO.MousePos.x = vMousePos.fX;
    rstIO.MousePos.y = vMousePos.fY;
    rstIO.MouseWheel = orxMouse_GetWheelDelta();
    for(int i = 0; i < IM_ARRAYSIZE(rstIO.MouseDown); i++)
    {
      rstIO.MouseDown[i] = orxMouse_IsButtonPressed((orxMOUSE_BUTTON)i) ? true : false;
    }

    for(unsigned int i = 0; i < orxKEYBOARD_KEY_NUMBER; i++)
    {
      rstIO.KeysDown[i] = orxKeyboard_IsKeyPressed((orxKEYBOARD_KEY)i) ? true : false;
    }
    rstIO.KeyCtrl   = (orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_LCTRL) || orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_RCTRL)) ? true : false;
    rstIO.KeyShift  = (orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_LSHIFT) || orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_RSHIFT)) ? true : false;
    rstIO.KeyAlt    = (orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_LALT) || orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_RALT)) ? true : false;
    rstIO.KeySuper  = (orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_LSYSTEM) || orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_RSYSTEM)) ? true : false;

    rstIO.NavInputs[ImGuiNavInput_Activate]     = orxJoystick_IsButtonPressed(orxJOYSTICK_BUTTON_A_1) ? 1.0f : 0.0f;
    rstIO.NavInputs[ImGuiNavInput_Cancel]       = orxJoystick_IsButtonPressed(orxJOYSTICK_BUTTON_B_1) ? 1.0f : 0.0f;
    rstIO.NavInputs[ImGuiNavInput_Input]        = orxJoystick_IsButtonPressed(orxJOYSTICK_BUTTON_Y_1) ? 1.0f : 0.0f;
    rstIO.NavInputs[ImGuiNavInput_Menu]         = orxJoystick_IsButtonPressed(orxJOYSTICK_BUTTON_X_1) ? 1.0f : 0.0f;
    rstIO.NavInputs[ImGuiNavInput_DpadLeft]     = orxJoystick_IsButtonPressed(orxJOYSTICK_BUTTON_LEFT_1) ? 1.0f : 0.0f;
    rstIO.NavInputs[ImGuiNavInput_DpadRight]    = orxJoystick_IsButtonPressed(orxJOYSTICK_BUTTON_RIGHT_1) ? 1.0f : 0.0f;
    rstIO.NavInputs[ImGuiNavInput_DpadUp]       = orxJoystick_IsButtonPressed(orxJOYSTICK_BUTTON_UP_1) ? 1.0f : 0.0f;
    rstIO.NavInputs[ImGuiNavInput_DpadDown]     = orxJoystick_IsButtonPressed(orxJOYSTICK_BUTTON_DOWN_1) ? 1.0f : 0.0f;
    rstIO.NavInputs[ImGuiNavInput_LStickLeft]   = orxJoystick_GetAxisValue(orxJOYSTICK_AXIS_LX_1) <= -0.3f ? 1.0f : 0.0f;
    rstIO.NavInputs[ImGuiNavInput_LStickRight]  = orxJoystick_GetAxisValue(orxJOYSTICK_AXIS_LX_1) >= 0.3f ? 1.0f : 0.0f;
    rstIO.NavInputs[ImGuiNavInput_LStickUp]     = orxJoystick_GetAxisValue(orxJOYSTICK_AXIS_LY_1) <= -0.3f ? 1.0f : 0.0f;
    rstIO.NavInputs[ImGuiNavInput_LStickDown]   = orxJoystick_GetAxisValue(orxJOYSTICK_AXIS_LY_1) >= 0.3f ? 1.0f : 0.0f;
    rstIO.NavInputs[ImGuiNavInput_FocusPrev]    = orxJoystick_IsButtonPressed(orxJOYSTICK_BUTTON_LBUMPER_1) ? 1.0f : 0.0f;
    rstIO.NavInputs[ImGuiNavInput_FocusNext]    = orxJoystick_IsButtonPressed(orxJOYSTICK_BUTTON_RBUMPER_1) ? 1.0f : 0.0f;
    rstIO.NavInputs[ImGuiNavInput_TweakSlow]    = orxJoystick_GetAxisValue(orxJOYSTICK_AXIS_LTRIGGER_1) >= 0.3f ? 1.0f : 0.0f;
    rstIO.NavInputs[ImGuiNavInput_TweakFast]    = orxJoystick_GetAxisValue(orxJOYSTICK_AXIS_RTRIGGER_1) >= 0.3f ? 1.0f : 0.0f;

    ImGui::NewFrame();

    if(rstIO.WantTextInput)
    {
      orxKeyboard_Show(orxTRUE);
      rstIO.AddInputCharactersUTF8(orxKeyboard_ReadString());
    }
    else
    {
      orxKeyboard_Show(orxFALSE);
    }
    if(rstIO.WantSetMousePos)
    {
      orxVECTOR vMousePos = {{rstIO.MousePos.x}, {rstIO.MousePos.y}};
      orxMouse_SetPosition(&vMousePos);
    }
    if(rstIO.WantCaptureMouse)
    {
      orxInput_SetTypeFlags(orxINPUT_KU32_FLAG_TYPE_NONE, orxINPUT_GET_FLAG(orxINPUT_TYPE_MOUSE_BUTTON) | orxINPUT_GET_FLAG(orxINPUT_TYPE_MOUSE_AXIS));
    }
    else
    {
      orxInput_SetTypeFlags(orxINPUT_GET_FLAG(orxINPUT_TYPE_MOUSE_BUTTON) | orxINPUT_GET_FLAG(orxINPUT_TYPE_MOUSE_AXIS), orxINPUT_KU32_FLAG_TYPE_NONE);
    }
    if(rstIO.WantCaptureKeyboard)
    {
      orxInput_SetTypeFlags(orxINPUT_KU32_FLAG_TYPE_NONE, orxINPUT_GET_FLAG(orxINPUT_TYPE_KEYBOARD_KEY));
    }
    else
    {
      orxInput_SetTypeFlags(orxINPUT_GET_FLAG(orxINPUT_TYPE_KEYBOARD_KEY), orxINPUT_KU32_FLAG_TYPE_NONE);
    }
  }

  return orxSTATUS_SUCCESS;
}

static orxSTATUS orxFASTCALL orxImGui_EndFrame(const orxEVENT *_pstEvent)
{
  if(sbInFrame)
  {
    sbInFrame = orxFALSE;

    ImGui::EndFrame();
    ImGui::Render();
    ImDrawData *pstDrawData = ImGui::GetDrawData();

    orxBITMAP *pstScreen = orxDisplay_GetScreenBitmap();
    orxDisplay_SetDestinationBitmaps(&pstScreen, 1);

    orxDISPLAY_MESH stMesh = {};
    stMesh.ePrimitive = orxDISPLAY_PRIMITIVE_TRIANGLES;

    for(int i = 0; i < pstDrawData->CmdListsCount; i++)
    {
      ImDrawList *pstDrawList = pstDrawData->CmdLists[i];

      stMesh.astVertexList    = (orxDISPLAY_VERTEX *)pstDrawList->VtxBuffer.Data;
      stMesh.u32VertexNumber  = pstDrawList->VtxBuffer.size();
      stMesh.au16IndexList    = pstDrawList->IdxBuffer.Data;

      for(int j = 0; j < pstDrawList->CmdBuffer.Size; j++)
      {
        ImDrawCmd &rstCommand = pstDrawList->CmdBuffer[j];

        if(rstCommand.UserCallback)
        {
          rstCommand.UserCallback(pstDrawList, &rstCommand);
        }
        else
        {
          stMesh.u32IndexNumber = rstCommand.ElemCount;
          orxDisplay_SetBitmapClipping(orxNULL, orxF2U(rstCommand.ClipRect.x - pstDrawData->DisplayPos.x), orxF2U(rstCommand.ClipRect.y - pstDrawData->DisplayPos.y), orxF2U(rstCommand.ClipRect.z - pstDrawData->DisplayPos.x), orxF2U(rstCommand.ClipRect.w - pstDrawData->DisplayPos.y));
          orxDisplay_DrawMesh(&stMesh, (orxBITMAP *)rstCommand.TextureId, orxDISPLAY_SMOOTHING_ON, orxDISPLAY_BLEND_MODE_ALPHA);
        }
        stMesh.au16IndexList += rstCommand.ElemCount;
      }
    }
  }

  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL orxImGui_Init()
{
  ImGui::SetAllocatorFunctions(&orxImGui_Allocate, &orxImGui_Free);
  ImGui::CreateContext();
  ImGuiIO& rstIO                    = ImGui::GetIO();
  rstIO.BackendFlags                = ImGuiBackendFlags_HasMouseCursors | ImGuiBackendFlags_HasSetMousePos | ImGuiBackendFlags_HasGamepad;
  rstIO.ConfigFlags                 = ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad | ImGuiConfigFlags_NavEnableSetMousePos | ImGuiConfigFlags_DockingEnable;
  rstIO.KeyMap[ImGuiKey_Tab]        = orxKEYBOARD_KEY_TAB;
  rstIO.KeyMap[ImGuiKey_LeftArrow]  = orxKEYBOARD_KEY_LEFT;
  rstIO.KeyMap[ImGuiKey_RightArrow] = orxKEYBOARD_KEY_RIGHT;
  rstIO.KeyMap[ImGuiKey_UpArrow]    = orxKEYBOARD_KEY_UP;
  rstIO.KeyMap[ImGuiKey_DownArrow]  = orxKEYBOARD_KEY_DOWN;
  rstIO.KeyMap[ImGuiKey_PageUp]     = orxKEYBOARD_KEY_PAGE_UP;
  rstIO.KeyMap[ImGuiKey_PageDown]   = orxKEYBOARD_KEY_PAGE_DOWN;
  rstIO.KeyMap[ImGuiKey_Home]       = orxKEYBOARD_KEY_HOME;
  rstIO.KeyMap[ImGuiKey_End]        = orxKEYBOARD_KEY_END;
  rstIO.KeyMap[ImGuiKey_Insert]     = orxKEYBOARD_KEY_INSERT;
  rstIO.KeyMap[ImGuiKey_Delete]     = orxKEYBOARD_KEY_DELETE;
  rstIO.KeyMap[ImGuiKey_Backspace]  = orxKEYBOARD_KEY_BACKSPACE;
  rstIO.KeyMap[ImGuiKey_Space]      = orxKEYBOARD_KEY_SPACE;
  rstIO.KeyMap[ImGuiKey_Enter]      = orxKEYBOARD_KEY_ENTER;
  rstIO.KeyMap[ImGuiKey_Escape]     = orxKEYBOARD_KEY_ESCAPE;
  rstIO.KeyMap[ImGuiKey_A]          = orxKEYBOARD_KEY_A;
  rstIO.KeyMap[ImGuiKey_C]          = orxKEYBOARD_KEY_C;
  rstIO.KeyMap[ImGuiKey_V]          = orxKEYBOARD_KEY_V;
  rstIO.KeyMap[ImGuiKey_X]          = orxKEYBOARD_KEY_X;
  rstIO.KeyMap[ImGuiKey_Y]          = orxKEYBOARD_KEY_Y;
  rstIO.KeyMap[ImGuiKey_Z]          = orxKEYBOARD_KEY_Z;

  int iWidth, iHeight;
  unsigned char *pcPixels;
  rstIO.Fonts->GetTexDataAsRGBA32(&pcPixels, &iWidth, &iHeight);
  orxBITMAP *pstBitmap = orxDisplay_CreateBitmap(iWidth, iHeight);
  orxDisplay_SetBitmapData(pstBitmap, pcPixels, iWidth * iHeight * 4);
  rstIO.Fonts->TexID = (void *)pstBitmap;

  rstIO.MouseDrawCursor = true;
  orxMouse_ShowCursor(orxFALSE);

  rstIO.GetClipboardTextFn = &orxImGui_GetClipboard;
  rstIO.SetClipboardTextFn = &orxImGui_SetClipboard;

  orxEvent_AddHandler(orxEVENT_TYPE_RENDER, &orxImGui_BeginFrame);
  orxEvent_SetHandlerIDFlags(&orxImGui_BeginFrame, orxEVENT_TYPE_RENDER, orxNULL, orxEVENT_GET_FLAG(orxRENDER_EVENT_STOP), orxEVENT_KU32_MASK_ID_ALL);
  orxEvent_AddHandler(orxEVENT_TYPE_RENDER, &orxImGui_EndFrame);
  orxEvent_SetHandlerIDFlags(&orxImGui_EndFrame, orxEVENT_TYPE_RENDER, orxNULL, orxEVENT_GET_FLAG(orxRENDER_EVENT_PROFILER_START), orxEVENT_KU32_MASK_ID_ALL);

  orxImGui_BeginFrame(orxNULL);

  return orxSTATUS_SUCCESS;
}

void orxFASTCALL orxImGui_Exit()
{
  orxDisplay_DeleteBitmap((orxBITMAP *)ImGui::GetIO().Fonts->TexID);
  ImGui::DestroyContext();
}

#endif // orxIMGUI_HEADER_ONLY
