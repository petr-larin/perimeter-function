#include "access.hpp"

int WINAPI
wWinMain(
	_In_ HINSTANCE hInst,
	_In_opt_ HINSTANCE,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow
)
{
	static Access app{ hInst, lpCmdLine, nCmdShow };

	try
	{
		app.InitApp();
	}
	catch (const GeneralError& exc)
	{
		app.Catch(exc);
	}
	catch (const std::bad_alloc& exc)
	{
		app.CatchBadAlloc(exc);
	}

	MSG msg{};
	BOOL iRet{};

	while ((iRet = GetMessageW(&msg, NULL, 0, 0)) != 0)
	{ 
		if (iRet == -1)
        {
			return 0;
		}
		else
        {
	        TranslateMessage(&msg);
		    DispatchMessageW(&msg);
		}
	}
 	return 0; 
}