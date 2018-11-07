#include "stdafx.h"
#include "GUI.h"

void GUI::Draw()
{
	DrawCameraEditor();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void GUI::DrawCameraEditor()
{
	ImGui::Begin("Camera Editor");   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
	ImGui::Text("Camera Position");
	float positionX;
	float positionY;
	float positionZ;
	ImGui::SliderFloat("POS X", &positionX, 0.0f, 1.0f);
	ImGui::SliderFloat("POS Y", &positionY, 0.0f, 1.0f);
	ImGui::SliderFloat("POS Z", &positionZ, 0.0f, 1.0f);
	ImGui::End();
}