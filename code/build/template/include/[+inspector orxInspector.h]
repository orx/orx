//! Includes

#ifndef _orxINSPECTOR_H_
#define _orxINSPECTOR_H_

#include "orxImGui.h"


//! Prototypes

void                                        orxInspector_Init();
void                                        orxInspector_Exit();
orxSTATUS                                   orxInspector_RegisterObject(orxOBJECT *_pstObject);
orxSTATUS                                   orxInspector_UnregisterObject(orxOBJECT *_pstObject);
orxBOOL                                     orxInspector_IsObjectRegistered(const orxOBJECT *_pstObject);


//! Defines

#define orxINSPECTOR_KZ_CONFIG_SECTION      "Inspector"
#define orxINSPECTOR_KZ_CONFIG_RUNTIME      "InspectorRuntime"

#define orxINSPECTOR_KZ_CONFIG_SHADER       "Shader"
#define orxINSPECTOR_KZ_CONFIG_DEBUG_FIELDS "InspectorFields"
#define orxINSPECTOR_KZ_CONFIG_WINDOW_SIZE  "WindowSize"

#define orxINSPECTOR_KZ_INPUT_SET           "orx:input:inspector"
#define orxINSPECTOR_KZ_INPUT_INSPECT       "Inspect"

#define orxINSPECTOR_KV_DEFAULT_WINDOW_SIZE orx2F(500.0f), orx2F(600.0f), orxFLOAT_0

#define orxINSPECTOR_KZ_SHADER_HIGHLIGHT    "highlight"
#define orxINSPECTOR_KZ_SHADER_CONFIG                                                                 \
"UseCustomParam  = true\n"                                                                            \
"ParamList       = texture # time # highlight # width # pixel_size # width_speed\n"                   \
"time            = time 3.14\n"                                                                       \
"highlight       = (1, 1, 1)\n"                                                                       \
"width           = 5.0\n"                                                                             \
"pixel_size      = 500\n"                                                                             \
"width_speed     = 1.0\n"                                                                             \
"Code            = \""                                                                                \
"void main() {\n"                                                                                     \
"  // Outline color\n"                                                                                \
"  vec4 outline_color = vec4(highlight, 1.0);\n"                                                      \
"\n"                                                                                                  \
"  // Texture bounds\n"                                                                               \
"	vec2 texture_min = vec2(texture_left, texture_top);\n"                                              \
"	vec2 texture_max = vec2(texture_right, texture_bottom);\n"                                          \
"	vec2 texture_size = texture_max - texture_min;\n"                                                   \
"\n"                                                                                                  \
"  // Position and texture color\n"                                                                   \
"  vec2 pixel_pos = gl_TexCoord[0].xy;\n"                                                             \
"  vec4 pixel_color = texture2D(texture, pixel_pos);\n"                                               \
"\n"                                                                                                  \
"  float _width = width; // + (sin(time*width_speed) + 1.0) * 5.0;\n"                                 \
"\n"                                                                                                  \
"  // Unit of sampling in each direction\n"                                                           \
"  vec2 unit = vec2(1.0/float(pixel_size)) * texture_size;\n"                                         \
"\n"                                                                                                  \
"  // Balance between the outline color and texture color\n"                                          \
"  float balance = 0.0;\n"                                                                            \
"  if (pixel_color.a != 0.0) {\n"                                                                     \
"    // Sample in a box around the current texture element\n"                                         \
"    for (float x = -ceil(_width); x <= ceil(_width); x++) {\n"                                       \
"      for (float y = -ceil(_width); y <= ceil(_width); y++) {\n"                                     \
"        vec2 offset_pos = pixel_pos + vec2(x*unit.x, y*unit.y);\n"                                   \
"        vec4 offset_color = vec4(0.0);\n"                                                            \
"        if (offset_pos.x >= texture_min.x && offset_pos.x <= texture_max.x &&\n"                     \
"            offset_pos.y >= texture_min.y && offset_pos.y <= texture_max.y) {\n"                     \
"          offset_color = texture2D(texture, offset_pos);\n"                                          \
"        }\n"                                                                                         \
"        if (offset_color.a != 0.0 && gl_Color.a != 0.0 || (x==0.0 && y==0.0)) {\n"                   \
"          continue;\n"                                                                               \
"        }\n"                                                                                         \
"        balance += outline_color.a / (pow(x,2)+pow(y,2)) * (1.0-pow(2.0, -_width));\n"               \
"        if (balance > 1.0 || balance < 0.0) {\n"                                                     \
"          balance = 1.0;\n"                                                                          \
"        }\n"                                                                                         \
"      }\n"                                                                                           \
"    }\n"                                                                                             \
"  }\n"                                                                                               \
"  // Use the following line to have the outline color blend at the edges\n"                          \
"  // gl_FragColor = mix(pixel_color * gl_Color, pixel_color * gl_Color * outline_color, balance);\n" \
"  // Use the following line to have the outline color only at the edges\n"                           \
"  gl_FragColor = mix(pixel_color * gl_Color, outline_color, balance);\n"                             \
"}\""

#define orxINSPECTOR_KU32_TEXT_BANK_SIZE    256
#define orxINSPECTOR_KU32_TEXT_BUFFER_SIZE  1024

#define orxINSPECTOR_KU32_TABLE_FLAGS       (ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp)


#ifdef orxINSPECTOR_IMPL


//! Variables / Structures

typedef struct __orxINSPECTOR_t
{
  const orxSTRING zShader;
  orxBANK        *pstTextBank;
  orxBOOL         bInit;

} orxINSPECTOR;

static orxINSPECTOR sstInspector;


//! Helpers

const orxSTRING orxInspector_PrintGUID(const orxOBJECT *_pstObject, const orxSTRING _zPrefix = orxNULL)
{
  static orxCHAR sacBuffer[orxINSPECTOR_KU32_TEXT_BUFFER_SIZE];
  orxString_NPrint(sacBuffer, sizeof(sacBuffer), "%s%s0x%016llX",
                   (_zPrefix != orxNULL) ? _zPrefix : orxSTRING_EMPTY,
                   (_zPrefix != orxNULL) ? "##" : orxSTRING_EMPTY,
                   orxStructure_GetGUID(orxSTRUCTURE(_pstObject)));

  // Done!
  return sacBuffer;
}

orxVECTOR *orxInspector_GetHighlightColor(const orxOBJECT *_pstObject, orxVECTOR *_pvColor)
{
  orxCOLOR  stColor;
  orxU64    u64GUID;

  // Pick a color based on the object's GUID
  u64GUID = orxStructure_GetGUID(_pstObject);
  orxVector_Set(&(stColor.vHSL), (orxMath_Sin((orxFLOAT)(u64GUID & 0xFFFFFFFF)) + 1.0f) / 2.0f, orxFLOAT_1, orx2F(0.7f));
  stColor.fAlpha = orxFLOAT_1;
  orxColor_FromHSLToRGB(&stColor, &stColor);

  // Done!
  return orxVector_Copy(_pvColor, &(stColor.vRGB));
}


//! Code

void orxInspector_ShowConfigRow(const orxSTRING _zKey, orxBOOL _bForceUpdate = orxFALSE, const orxSTRING _zSection = orxNULL)
{
  ImGui::TableNextRow();

  // Draw the section column
  if(_zSection != orxNULL)
  {
    ImGui::TableNextColumn();
    ImGui::Text("%s", _zSection);
  }

  // Draw the key column
  ImGui::TableNextColumn();
  ImGui::Text("%s", _zKey);

  // Draw the value column
  ImGui::TableNextColumn();
  ImGui::PushID(_zKey);
  orxBOOL bHasValue = orxConfig_HasValueNoCheck(_zKey);
  if((bHasValue != orxFALSE) && (orxConfig_IsList(_zKey) != orxFALSE))
  {
    orxBOOL bNeedUpdate = _bForceUpdate;
    for(orxU32 i = 0, iCount = orxConfig_GetListCount(_zKey); i < iCount; i++)
    {
      // Push a unique ID for each input
      ImGui::PushID(i);

      // Initialize buffer with the current content
      orxCHAR *acBuffer = (orxCHAR *)orxBank_Allocate(sstInspector.pstTextBank);
      orxString_NPrint(acBuffer, orxINSPECTOR_KU32_TEXT_BUFFER_SIZE, "%s", orxConfig_GetListString(_zKey, i));

      // Input text widget
      ImGui::InputText("", acBuffer, orxINSPECTOR_KU32_TEXT_BUFFER_SIZE);

      // Track if the widget has received edits and lost focus
      if(ImGui::IsItemDeactivated())
      {
        bNeedUpdate = orxTRUE;
      }

      // Input and button on the same line
      ImGui::SameLine();

      // Button to remove this list entry
      if(ImGui::Button("Remove"))
      {
        orxBank_Free(sstInspector.pstTextBank, acBuffer);
        bNeedUpdate = orxTRUE;
      }

      ImGui::PopID();
    }

    // Widget to add an entry to the list
    if(ImGui::Button("Add list entry"))
    {
      // Request update
      bNeedUpdate = orxTRUE;

      // Add an empty string as a placeholder for the new entry
      *(orxCHAR *)orxBank_Allocate(sstInspector.pstTextBank) = orxCHAR_NULL;
    }

    // Need update?
    if(bNeedUpdate != orxFALSE)
    {
      // Update the config list
      const orxSTRING *azValues = (const orxSTRING *)alloca(orxBank_GetCount(sstInspector.pstTextBank) * sizeof(orxSTRING));
      orxCHAR *acBuffer;
      orxU32 u32Index;
      for(u32Index = 0, acBuffer = (orxCHAR *)orxBank_GetNext(sstInspector.pstTextBank, orxNULL);
          acBuffer != orxNULL;
          u32Index++, acBuffer = (orxCHAR *)orxBank_GetNext(sstInspector.pstTextBank, acBuffer))
      {
        azValues[u32Index] = acBuffer;
      }
      orxConfig_SetListString(_zKey, azValues, orxBank_GetCount(sstInspector.pstTextBank));
    }

    // Clear all buffers
    orxBank_Clear(sstInspector.pstTextBank);
  }
  else
  {
    orxCHAR acBuffer[orxINSPECTOR_KU32_TEXT_BUFFER_SIZE];
    if(bHasValue != orxFALSE)
    {
      orxSTRING zValue = orxConfig_DuplicateRawValue(_zKey);
      orxString_NPrint(acBuffer, sizeof(acBuffer), "%s", zValue);
      orxString_Delete(zValue);
    }
    else
    {
      *acBuffer = orxCHAR_NULL;
    }
    if(ImGui::InputText("", acBuffer, sizeof(acBuffer)))
    {
      orxConfig_SetString(_zKey, acBuffer);
    }
  }
  ImGui::PopID();

  // Done!
  return;
}

void orxInspector_ShowObjectGUID(const orxOBJECT *_pstObject)
{
  // Get object guid
  const orxSTRING zGUID = orxInspector_PrintGUID(_pstObject);
  ImGui::TextUnformatted(zGUID);
  ImGui::SameLine();

  // Make the guid easy to copy to the clipboard for use in commands
  if(ImGui::Button("Copy"))
  {
    ImGui::SetClipboardText(zGUID);
  }

  // Done!
  return;
}

void orxInspector_ShowObjectHeader()
{
  // Table header initialization
  ImGui::TableSetupColumn("Name");
  ImGui::TableSetupColumn("GUID");
  ImGui::TableSetupColumn("Inspector");
  ImGui::TableHeadersRow();

  // Done!
  return;
}

void orxInspector_ShowObjectButton(orxOBJECT *_pstObject)
{
  ImGui::TableNextColumn();
  ImGui::TextUnformatted(orxObject_GetName(_pstObject));
  ImGui::TableNextColumn();
  orxInspector_ShowObjectGUID(_pstObject);
  ImGui::TableNextColumn();
  if(ImGui::Button("Inspect"))
  {
    orxInspector_RegisterObject(_pstObject);
  }

  // Done!
  return;
}

void orxInspector_ShowParent(const orxOBJECT *_pstObject)
{
  orxOBJECT *pstParent = orxOBJECT(orxObject_GetParent(_pstObject));
  if(pstParent != orxNULL)
  {
    if(ImGui::CollapsingHeader("Parent"))
    {
      ImGui::PushID(pstParent);

      if(ImGui::BeginTable("Parent", 3, orxINSPECTOR_KU32_TABLE_FLAGS))
      {
        orxInspector_ShowObjectHeader();
        orxInspector_ShowObjectButton(pstParent);
      }
      ImGui::EndTable();

      ImGui::PopID();
    }
  }

  // Done!
  return;
}

void orxInspector_ShowSiblings(const orxOBJECT *_pstObject)
{
  orxOBJECT *pstParent = orxOBJECT(orxObject_GetParent(_pstObject));
  if(pstParent != orxNULL)
  {
    orxOBJECT *pstFirstSibling = orxObject_GetChild(pstParent);
    if(pstFirstSibling == _pstObject)
    {
      pstFirstSibling = orxObject_GetSibling(pstFirstSibling);
    }
    if(pstFirstSibling != orxNULL)
    {
      if(ImGui::CollapsingHeader("Siblings"))
      {
        if(ImGui::BeginTable("Siblings", 3, orxINSPECTOR_KU32_TABLE_FLAGS))
        {
          orxInspector_ShowObjectHeader();
          for(orxOBJECT *pstSibling = pstFirstSibling;
              pstSibling != orxNULL;
              pstSibling = orxObject_GetSibling(pstSibling))
          {
            if(pstSibling != _pstObject)
            {
              ImGui::PushID(pstSibling);
              orxInspector_ShowObjectButton(pstSibling);
              ImGui::PopID();
            }
          }
        }
        ImGui::EndTable();
      }
    }
  }

  // Done!
  return;
}

void orxInspector_ShowChildren(const orxOBJECT *_pstObject)
{
  orxOBJECT *pstChild = orxObject_GetChild(_pstObject);
  if(pstChild != orxNULL)
  {
    if(ImGui::CollapsingHeader("Children"))
    {
      if(ImGui::BeginTable("Children", 3, orxINSPECTOR_KU32_TABLE_FLAGS))
      {
        orxInspector_ShowObjectHeader();
        for(; pstChild != orxNULL; pstChild = orxObject_GetSibling(pstChild))
        {
          ImGui::PushID(pstChild);
          orxInspector_ShowObjectButton(pstChild);
          ImGui::PopID();
        }
      }
      ImGui::EndTable();
    }
  }

  // Done!
  return;
}

void orxInspector_ShowStatus(const orxOBJECT *_pstObject)
{
  if(_pstObject != orxNULL)
  {
    if(ImGui::CollapsingHeader("Status", ImGuiTreeNodeFlags_DefaultOpen))
    {
      if(ImGui::BeginTable("Status", 2, orxINSPECTOR_KU32_TABLE_FLAGS))
      {
        // GUID
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("GUID");
        ImGui::TableNextColumn();
        orxInspector_ShowObjectGUID(_pstObject);

        // Group
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Group");
        ImGui::TableNextColumn();
        ImGui::Text("%s", orxString_GetFromID(orxObject_GetGroupID(_pstObject)));

        // Life time
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Life time");
        ImGui::TableNextColumn();
        ImGui::Text("%0.1f", orxObject_GetLifeTime(_pstObject));

        // Active time
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Active time");
        ImGui::TableNextColumn();
        ImGui::Text("%0.1f", orxObject_GetActiveTime(_pstObject));

        // Current animation
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Animation");
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(orxObject_GetCurrentAnim(_pstObject));

        // World position
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Position");
        ImGui::TableNextColumn();
        orxVECTOR position;
        orxObject_GetWorldPosition(_pstObject, &position);
        ImGui::Text("(%0.1f, %0.1f, %0.1f)", position.fX, position.fY, position.fZ);

        // Velocity
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Speed");
        ImGui::TableNextColumn();
        orxVECTOR speed;
        orxObject_GetSpeed(_pstObject, &speed);
        ImGui::Text("(%0.1f, %0.1f)", speed.fX, speed.fY);

        // Rotation
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Rotation");
        ImGui::TableNextColumn();
        ImGui::Text("%0.1f °", orxObject_GetRotation(_pstObject) * orxMATH_KF_RAD_TO_DEG);

        // Angular velocity
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Angular velocity");
        ImGui::TableNextColumn();
        ImGui::Text("%0.1f °/s", orxObject_GetAngularVelocity(_pstObject) * orxMATH_KF_RAD_TO_DEG);

        // Scale
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Scale");
        ImGui::TableNextColumn();
        orxVECTOR scale;
        orxObject_GetScale(_pstObject, &scale);
        ImGui::Text("(%0.2f, %0.2f)", scale.fX, scale.fY);

        // Flip state of the object
        orxBOOL bFlipX, bFlipY;
        orxObject_GetFlip(_pstObject, &bFlipX, &bFlipY);

        // Show information on scale
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Flip");
        ImGui::TableNextColumn();
        ImGui::Text("(%s, %s)", (bFlipX ? "Flip X" : "Normal X"), (bFlipY ? "Flip Y" : "Normal Y"));
      }
      ImGui::EndTable();
    }
  }

  // Done!
  return;
}

void orxInspector_ShowConfig(const orxSTRING _zSection)
{
  if(ImGui::CollapsingHeader("Config"))
  {
    // Track additional indentation levels from parent config sections
    float fIndentOffset = 5.0f;
    ImGui::Indent(fIndentOffset);

    // Object config section and its inheritance hierarchy
    if(ImGui::CollapsingHeader(_zSection))
    {
      // Create a table with one row per key/value pair in the section
      for(const orxSTRING zParent = _zSection;
          (zParent != orxNULL) && (*zParent != orxCHAR_NULL);
          zParent = orxConfig_GetParent(zParent))
      {
        ImGui::Indent(5.0f);
        fIndentOffset += 5.0f;

        if((zParent == _zSection)
        || (ImGui::CollapsingHeader(zParent, ImGuiTreeNodeFlags_DefaultOpen)))
        {
          if(ImGui::BeginTable(zParent, 2, orxINSPECTOR_KU32_TABLE_FLAGS))
          {
            // Show the config from the parent section too
            ImGui::PushID(zParent);
            orxConfig_PushSection(zParent);
            for(orxU32 i = 0, iCount = orxConfig_GetKeyCount(); i < iCount; i++)
            {
              const orxSTRING zKey = orxConfig_GetKey(i);
              orxInspector_ShowConfigRow(zKey, (orxString_Compare(zKey, orxINSPECTOR_KZ_CONFIG_DEBUG_FIELDS) == 0) ? orxTRUE : orxFALSE);
            }
            orxConfig_PopSection();
            ImGui::PopID();

            ImGui::EndTable();
          }
        }
      }
    }

    ImGui::Indent(-fIndentOffset);

    fIndentOffset = 5.0f;
    ImGui::Indent(fIndentOffset);

    // Other custom config values to show
    orxConfig_PushSection(_zSection);
    orxU32 u32ElementCount = orxConfig_GetListCount(orxINSPECTOR_KZ_CONFIG_DEBUG_FIELDS);
    if((u32ElementCount > 0)
    && ImGui::CollapsingHeader("Inspector Fields")
    && ImGui::BeginTable("Inspector Fields", 3, orxINSPECTOR_KU32_TABLE_FLAGS))
    {
      ImGui::PushID("Custom");

      ImGui::TableSetupColumn("Section");
      ImGui::TableSetupColumn("Key");
      ImGui::TableSetupColumn("Value");
      ImGui::TableHeadersRow();

      for(orxU32 u32Element = 0;
          u32Element < u32ElementCount;
          u32Element++)
      {
        const orxSTRING zPath = orxConfig_GetListString(orxINSPECTOR_KZ_CONFIG_DEBUG_FIELDS, u32Element);
        if(*zPath != orxCHAR_NULL)
        {
          orxS32 s32SeparatorIndex = orxString_SearchCharIndex(zPath, '.', 0);
          if(s32SeparatorIndex > 0)
          {
            orxCHAR acSection[1024];

            ImGui::PushID(u32Element);

            orxString_NPrint(acSection, sizeof(acSection), "%.*s", s32SeparatorIndex, zPath);
            orxConfig_PushSection(acSection);
            orxInspector_ShowConfigRow(zPath + s32SeparatorIndex + 1, orxFALSE, acSection);
            orxConfig_PopSection();

            ImGui::PopID();
          }
        }
      }
      ImGui::PopID();

      ImGui::EndTable();
    }
    orxConfig_PopSection();

    ImGui::Indent(-fIndentOffset);
  }

  // Done!
  return;
}

void orxInspector_ShowHighlightCheckbox(orxOBJECT *_pstObject)
{
  // Has a working graphic?
  if(orxObject_GetWorkingGraphic(_pstObject) != orxNULL)
  {
    orxVECTOR vColor;
    orxInspector_GetHighlightColor(_pstObject, &vColor);
    ImVec4 vHighlight = {vColor.fR, vColor.fG, vColor.fB, 1.0f};
    ImGui::TextColored(vHighlight, "Highlight");
    ImGui::SameLine();

    orxSHADERPOINTER *pstShaderPointer = orxOBJECT_GET_STRUCTURE(_pstObject, SHADERPOINTER);
    orxU32 u32HighlightShaderIndex = orxU32_UNDEFINED;
    if(pstShaderPointer != orxNULL)
    {
      for(orxU32 i = 0; i < orxSHADERPOINTER_KU32_SHADER_NUMBER; i++)
      {
        const orxSHADER *pstShader = orxShaderPointer_GetShader(pstShaderPointer, i);
        if((pstShader != orxNULL) && (orxString_Compare(orxShader_GetName(pstShader), sstInspector.zShader) == 0))
        {
          u32HighlightShaderIndex = i;
          break;
        }
      }
    }

    bool bHighlight = (u32HighlightShaderIndex != orxU32_UNDEFINED) ? true : false;
    if(ImGui::Checkbox("##highlight", &bHighlight))
    {
      if(bHighlight != false)
      {
        if(u32HighlightShaderIndex == orxU32_UNDEFINED)
        {
          orxObject_AddShader(_pstObject, sstInspector.zShader);
        }
      }
      else
      {
        if(u32HighlightShaderIndex != orxU32_UNDEFINED)
        {
          orxObject_RemoveShader(_pstObject, sstInspector.zShader);
        }
      }
    }
  }

  // Done!
  return;
}

void orxInspacter_ShowShaders(const orxOBJECT *_pstObject)
{
  orxBOOL bTableInitialized = orxFALSE;

  orxSHADERPOINTER *pstShaderPointer = orxOBJECT_GET_STRUCTURE(_pstObject, SHADERPOINTER);
  if(pstShaderPointer != orxNULL)
  {
    for(orxU32 i = 0; i < orxSHADERPOINTER_KU32_SHADER_NUMBER; i++)
    {
      const orxSHADER *pstShader = orxShaderPointer_GetShader(pstShaderPointer, i);
      if((pstShader != orxNULL) && (orxString_Compare(orxShader_GetName(pstShader), sstInspector.zShader) != 0))
      {
        if(bTableInitialized == orxFALSE)
        {
          if((!ImGui::CollapsingHeader("Shaders"))
          || (bTableInitialized = orxTRUE, !ImGui::BeginTable("Shaders", 2, orxINSPECTOR_KU32_TABLE_FLAGS)))
          {
            break;
          }
        }

        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Name");
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(orxShader_GetName(pstShader));
      }
    }
    if(bTableInitialized != orxFALSE)
    {
      ImGui::EndTable();
    }
  }

  // Done!
  return;
}

void orxInspector_ShowGraphic(const orxOBJECT *_pstObject)
{
  orxGRAPHIC     *pstGraphic  = orxObject_GetWorkingGraphic(_pstObject);
  orxTEXTURE     *pstTexture  = orxObject_GetWorkingTexture(_pstObject);
  const orxSTRING zText       = orxObject_GetTextString(_pstObject);
  if((pstGraphic != orxNULL)
  && ((pstTexture != orxNULL)
   || (*zText != orxCHAR_NULL)))
  {
    if(ImGui::CollapsingHeader("Graphic"))
    {
      // Setup table columns
      if(ImGui::BeginTable("Graphic", 2, orxINSPECTOR_KU32_TABLE_FLAGS))
      {
        // Show object graphic

        // Flip state of the graphic
        orxBOOL bFlipX, bFlipY;
        orxGraphic_GetFlip(pstGraphic, &bFlipX, &bFlipY);

        // Show information on scale
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Flip");
        ImGui::TableNextColumn();
        ImGui::Text("(%s, %s)", (bFlipX ? "Flip X" : "Normal X"), (bFlipY ? "Flip Y" : "Normal Y"));

        if(*zText != orxCHAR_NULL)
        {
          // Show object text

          // Identify type of graphic
          ImGui::TableNextColumn();
          ImGui::TextUnformatted("Text");

          // Show graphic content
          ImGui::TableNextColumn();
          ImGui::TextUnformatted(zText);
        }
        else
        {

          // Scale of graphic
          orxVECTOR vScale;
          orxObject_GetScale(_pstObject, &vScale);

          // Identify type of graphic
          ImGui::TableNextColumn();
          ImGui::TextUnformatted("Texture");

          // Show graphic content
          ImGui::TableNextColumn();
          // Get overall texture dimensions
          orxFLOAT fTextureWidth, fTextureHeight;
          orxTexture_GetSize(pstTexture, &fTextureWidth, &fTextureHeight);

          // Get the origin and size of the active graphic
          orxVECTOR vOrigin, vSize;
          orxGraphic_GetOrigin(pstGraphic, &vOrigin);
          orxGraphic_GetSize(pstGraphic, &vSize);

          // Find the location of the graphic within the overall texture in normalized coordinates
          ImVec2 avUVs[2];
          // Top left of the texture region
          avUVs[0].x = vOrigin.fX / fTextureWidth;
          avUVs[0].y = vOrigin.fY / fTextureHeight;
          // Lower right of the texture region
          avUVs[1].x = (vOrigin.fX + vSize.fX) / fTextureWidth;
          avUVs[1].y = (vOrigin.fY + vSize.fY) / fTextureHeight;

          // Swap UV coordinates if the object is flipped
          if(bFlipX)
          {
            float fTemp = avUVs[0].x;
            avUVs[0].x  = avUVs[1].x;
            avUVs[1].x  = fTemp;
          }
          if(bFlipY)
          {
            float fTemp = avUVs[0].y;
            avUVs[0].y  = avUVs[1].y;
            avUVs[1].y  = fTemp;
          }

          // Get the texture and display the portion that matches the object's graphic
          ImTextureID textureID = (ImTextureID)orxTexture_GetBitmap(pstTexture);
          ImGui::Image(textureID, {vSize.fX * vScale.fX, vSize.fY * vScale.fY}, avUVs[0], avUVs[1]);
        }
      }
      ImGui::EndTable();
    }
  }

  // Done!
  return;
}

void orxInspector_ShowObject(orxOBJECT *_pstObject)
{
  bool bRegistered = true;

  // Set a default size for the window
  orxVECTOR vSize = {orxINSPECTOR_KV_DEFAULT_WINDOW_SIZE};
  orxConfig_PushSection(orxINSPECTOR_KZ_CONFIG_SECTION);
  orxConfig_GetVector(orxINSPECTOR_KZ_CONFIG_WINDOW_SIZE, &vSize);
  orxConfig_PopSection();
  ImGui::SetNextWindowSize({vSize.fX, vSize.fY}, ImGuiCond_FirstUseEver);

  // Build a window title with the object name GUID to make the window's ID unique. The GUID is hidden.
  const orxSTRING zName = orxObject_GetName(_pstObject);
  if(ImGui::Begin(orxInspector_PrintGUID(_pstObject, zName), &bRegistered))
  {
    orxInspector_ShowHighlightCheckbox(_pstObject);
    orxInspector_ShowStatus(_pstObject);
    orxInspector_ShowConfig(zName);
    orxInspector_ShowGraphic(_pstObject);
    orxInspacter_ShowShaders(_pstObject);
    orxInspector_ShowParent(_pstObject);
    orxInspector_ShowSiblings(_pstObject);
    orxInspector_ShowChildren(_pstObject);
  }
  ImGui::End();

  if(bRegistered == false)
  {
    orxInspector_UnregisterObject(_pstObject);
    orxObject_RemoveShader(_pstObject, sstInspector.zShader);
  }

  // Done!
  return;
}

void orxInspector_CommandRegisterObject(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  // Update result
  _pstResult->u64Value = orxU64_UNDEFINED;

  // Gets object
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  // Valid?
  if(pstObject != orxNULL)
  {
    // Register the object
    if(orxInspector_RegisterObject(pstObject) != orxSTATUS_FAILURE)
    {
      // Update result
      _pstResult->u64Value = _astArgList[0].u64Value;
    }
  }

  // Done!
  return;
}

void orxInspector_CommandUnregisterObject(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  // Update result
  _pstResult->u64Value = orxU64_UNDEFINED;

  // Gets object
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  // Valid?
  if(pstObject != orxNULL)
  {
    // Unregister the object
    if(orxInspector_UnregisterObject(pstObject) != orxSTATUS_FAILURE)
    {
      // Update result
      _pstResult->u64Value = _astArgList[0].u64Value;
    }
  }

  // Done!
  return;
}

void orxInspector_CommandIsObjectRegistered(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  // Update result
  _pstResult->bValue = orxFALSE;

  // Gets object
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  // Update result
  _pstResult->bValue = orxInspector_IsObjectRegistered(pstObject);

  // Done!
  return;
}

orxSTATUS orxInspector_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Get the event payload
  orxSHADER_EVENT_PAYLOAD *pstPayload = (orxSHADER_EVENT_PAYLOAD *)_pstEvent->pstPayload;

  // Inspector highlight?
  if((orxString_Compare(pstPayload->zShaderName, sstInspector.zShader) == 0)
  && (orxString_Compare(pstPayload->zParamName, orxINSPECTOR_KZ_SHADER_HIGHLIGHT) == 0))
  {
    // Set the highlight color for the shader
    orxInspector_GetHighlightColor(orxOBJECT(_pstEvent->hSender), &(pstPayload->vValue));
  }

  // Done!
  return eResult;
}

void orxInspector_Update(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
  // Push input set
  orxInput_PushSet(orxINSPECTOR_KZ_INPUT_SET);

  // Should inspect object?
  if(orxInput_HasBeenActivated(orxINSPECTOR_KZ_INPUT_INSPECT))
  {
    orxVECTOR vMousePosition;

    // Pick object
    orxRender_GetWorldPosition(orxMouse_GetPosition(&vMousePosition), orxNULL, &vMousePosition);
    orxOBJECT *pstObject = orxObject_Pick(&vMousePosition, orxSTRINGID_UNDEFINED);

    // Found?
    if(pstObject != orxNULL)
    {
      // Toggle its registration
      if(orxInspector_IsObjectRegistered(pstObject))
      {
        orxInspector_UnregisterObject(pstObject);
      }
      else
      {
        orxInspector_RegisterObject(pstObject);
      }
    }
  }

  // Pop input set
  orxInput_PopSet();

  // Inspect all registered objects
  orxConfig_PushSection(orxINSPECTOR_KZ_CONFIG_RUNTIME);
  for(orxU32 u32KeyIndex = 0, u32KeyCount = orxConfig_GetKeyCount(); u32KeyIndex < u32KeyCount; u32KeyIndex++)
  {
    const orxSTRING zKey  = orxConfig_GetKey(u32KeyIndex);
    orxU64 u64GUID        = orxU64_UNDEFINED;
    if(orxString_ToU64(zKey, &u64GUID, orxNULL) != orxSTATUS_FAILURE)
    {
      orxOBJECT *pstObject = orxOBJECT(orxStructure_Get(u64GUID));
      if(pstObject != orxNULL)
      {
        orxInspector_ShowObject(pstObject);
      }
      else
      {
        orxConfig_ClearValue(zKey);
      }
    }
  }
  orxConfig_PopSection();

  // Done!
  return;
}

void orxInspector_Init()
{
  // Not already initialized?
  if(sstInspector.bInit == orxFALSE)
  {
    // Should be activated?
    if(orxConfig_HasSection(orxINSPECTOR_KZ_CONFIG_SECTION) != orxFALSE)
    {
      const orxSTRING zShaderConfig = orxINSPECTOR_KZ_SHADER_CONFIG;
      orxCHAR acBuffer[2048];

      // Push config section
      orxConfig_PushSection(orxINSPECTOR_KZ_CONFIG_SECTION);

      // Load shader config
      orxS32 s32Written = orxString_NPrint(acBuffer, sizeof(acBuffer), "%s", zShaderConfig);
      orxASSERT(s32Written == orxString_GetLength(zShaderConfig));
      orxConfig_LoadFromMemory(acBuffer, sizeof(acBuffer));

      // Init variables
      orxMemory_Zero(&(sstInspector), sizeof(orxINSPECTOR));
      sstInspector.zShader      = (orxConfig_HasValue(orxINSPECTOR_KZ_CONFIG_SHADER) != orxFALSE) ? orxConfig_GetString(orxINSPECTOR_KZ_CONFIG_SHADER) : orxINSPECTOR_KZ_CONFIG_SECTION;

      // Create text bank
      sstInspector.pstTextBank  = orxBank_Create(orxINSPECTOR_KU32_TEXT_BANK_SIZE, orxINSPECTOR_KU32_TEXT_BUFFER_SIZE, orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_DEBUG);
      orxASSERT(sstInspector.pstTextBank != orxNULL);

      // Register clock callback to render inspector windows on each frame
      orxClock_Register(orxClock_Get(orxCLOCK_KZ_CORE), orxInspector_Update, orxNULL, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_LOWER);

      // Add event handler
      orxEvent_AddHandler(orxEVENT_TYPE_SHADER, orxInspector_EventHandler);
      orxEvent_SetHandlerIDFlags(orxInspector_EventHandler,
                                 orxEVENT_TYPE_SHADER,
                                 orxNULL,
                                 orxEVENT_GET_FLAG(orxSHADER_EVENT_SET_PARAM),
                                 orxEVENT_KU32_MASK_ID_ALL);

      // Register commands
      orxCOMMAND_REGISTER_CORE_COMMAND(Inspector, RegisterObject, "GUID", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});
      orxCOMMAND_REGISTER_CORE_COMMAND(Inspector, UnregisterObject, "GUID", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});
      orxCOMMAND_REGISTER_CORE_COMMAND(Inspector, IsObjectRegistered, "Registered?", orxCOMMAND_VAR_TYPE_BOOL, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});

      // Enable input set
      const orxSTRING zParent = orxConfig_GetParent(orxINSPECTOR_KZ_INPUT_SET);
      orxConfig_SetParent(orxINSPECTOR_KZ_INPUT_SET, orxINSPECTOR_KZ_CONFIG_SECTION);
      orxInput_EnableSet(orxINSPECTOR_KZ_INPUT_SET, orxTRUE);
      orxConfig_SetParent(orxINSPECTOR_KZ_INPUT_SET, zParent);

      // Update status
      sstInspector.bInit = orxTRUE;

      // Pop config section
      orxConfig_PopSection();
    }
  }

  // Done!
  return;
}

void orxInspector_Exit()
{
  // Was initialized?
  if(sstInspector.bInit != orxFALSE)
  {
    // Disable input set
    orxInput_EnableSet(orxINSPECTOR_KZ_INPUT_SET, orxFALSE);
    orxInput_ClearSet(orxINSPECTOR_KZ_INPUT_SET);

    // Unregister commands
    orxCOMMAND_UNREGISTER_CORE_COMMAND(Inspector, RegisterObject);
    orxCOMMAND_UNREGISTER_CORE_COMMAND(Inspector, UnregisterObject);
    orxCOMMAND_UNREGISTER_CORE_COMMAND(Inspector, IsObjectRegistered);

    // Remove event handler
    orxEvent_RemoveHandler(orxEVENT_TYPE_SHADER, orxInspector_EventHandler);

    // Unregister clock callback
    orxClock_Unregister(orxClock_Get(orxCLOCK_KZ_CORE), orxInspector_Update);

    // Clean runtime config section
    orxConfig_ClearSection(orxINSPECTOR_KZ_CONFIG_RUNTIME);

    // Delete text bank
    orxBank_Delete(sstInspector.pstTextBank);
    sstInspector.pstTextBank = orxNULL;

    // Update status
    sstInspector.bInit = orxFALSE;
  }

  // Done!
  return;
}

orxSTATUS orxInspector_RegisterObject(orxOBJECT *_pstObject)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  // Valid?
  if(_pstObject != orxNULL)
  {
    orxConfig_PushSection(orxINSPECTOR_KZ_CONFIG_RUNTIME);
    orxConfig_SetBool(orxInspector_PrintGUID(_pstObject), orxTRUE);
    orxConfig_PopSection();

    orxObject_AddShader(_pstObject, sstInspector.zShader);

    // Update status
    eResult = orxSTATUS_SUCCESS;
  }

  // Done!
  return eResult;
}

orxSTATUS orxInspector_UnregisterObject(orxOBJECT *_pstObject)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  // Valid?
  if(_pstObject != orxNULL)
  {
    orxConfig_PushSection(orxINSPECTOR_KZ_CONFIG_RUNTIME);
    orxConfig_ClearValue(orxInspector_PrintGUID(_pstObject));
    orxConfig_PopSection();

    orxObject_RemoveShader(_pstObject, sstInspector.zShader);

    // Update result
    eResult = orxSTATUS_SUCCESS;
  }

  // Done!
  return eResult;
}

orxBOOL orxInspector_IsObjectRegistered(const orxOBJECT *_pstObject)
{
  orxBOOL bResult = orxFALSE;

  // Valid?
  if(_pstObject != orxNULL)
  {
    const orxSTRING zGUID = orxInspector_PrintGUID(_pstObject);
    orxConfig_PushSection(orxINSPECTOR_KZ_CONFIG_RUNTIME);
    bResult = ((orxConfig_HasValueNoCheck(zGUID) != orxFALSE) && (orxConfig_GetBool(zGUID) != orxFALSE)) ? orxTRUE : orxFALSE;
    orxConfig_PopSection();
  }

  // Done!
  return bResult;
}

#endif // orxINSPECTOR_IMPL

#endif // _orxINSPECTOR_H_
