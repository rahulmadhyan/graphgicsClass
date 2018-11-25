#include "stdafx.h"
#include "GUI.h"

void GUI::Draw()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
