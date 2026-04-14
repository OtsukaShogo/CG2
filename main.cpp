#include<windows.h>

//windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE,_In_opt_ HINSTANCE,_In_ LPSTR,_In_ int){
//出力ウィンドウへの文字入力
	OutputDebugStringA("Hellow,DirectX!\n");

	return 0;
}