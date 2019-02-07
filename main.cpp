#include <cpp_redis/cpp_redis>
#include <tacopie/tacopie>

#include "resource1.h"
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include <d3d9.h>
#define DIRECTINPUT_VERSION 0x0800
#include <functional>
#include <iterator>
#include <numeric>
#include <dinput.h>
#include <tchar.h>
#include <cstdio>
#include <string>
#include <chrono>
#include <ctime>
#include <iostream>
#include <thread>
#include <vector>
#include <map>
#include <set>
#include <regex>
#include <algorithm> //for indexOf
#include <signal.h>

#include "globals.h"
#include "times.h"

#include <stdlib.h>
#include <time.h>

#include "comdata.h"

#define Gui ImGui



// https://stackoverflow.com/questions/33336368/c-win32-loadstring-wrapper#33336980
std::wstring LoadStringW(unsigned int id)
{
    const wchar_t* p = nullptr;
    int len = ::LoadStringW(nullptr, id, reinterpret_cast<LPWSTR>(&p), 0);
    if (len > 0)
    {
        return std::wstring(p, static_cast<size_t>(len));
    }
    // Return empty string; optionally replace with throwing an exception.
    return std::wstring();
}

template<typename T>
auto indexOf(std::vector<T> vec, T item) {
    auto ret = (std::find(vec.begin(), vec.end(), item) - vec.begin());
    return ret == (int)vec.size() ? -1 : ret;
}

namespace Gui
{
    std::vector<std::string> vec3;// = { "tet","ert","dfg","michalis" };  //because of extern in ihgui.h

    static auto vector_getter = [](void* vec, int idx, const char** out_text)
    {
        auto& vector = *static_cast<std::vector<std::string>*>(vec);
        if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
        *out_text = vector.at(idx).c_str();
        return true;
    };

    bool Combo(const char* label, int* currIndex, std::vector<std::string>& values)
    {
        if (values.empty()) { return false; }
        return Combo(label, currIndex, vector_getter,
            static_cast<void*>(&values), values.size());
    }

    bool ListBox(const char* label, int* currIndex, std::vector<std::string>& values, int length = 10)
    {
        if (values.empty()) { return false; }
        return ListBox(label, currIndex, vector_getter,
            static_cast<void*>(&values), values.size(), length);
    }

}

//https://stackoverflow.com/questions/1041620/whats-the-most-efficient-way-to-erase-duplicates-and-sort-a-vector
template <class T>
void RemoveDuplicatesInVector(std::vector<T> & vec)
{
    std::set<T> values;
    vec.erase(std::remove_if(vec.begin(), vec.end(), [&](const T & value) { return !values.insert(value).second; }), vec.end());
}

void println(const std::string& str1) {
    printf("%s\n", str1.c_str());
}

void sigint_handler(int) {
    should_exit.notify_all();
}


void task1(std::string inpstr = "") { //pubsub
                                      //cpp_redis::active_logger = std::unique_ptr<cpp_redis::logger>(new cpp_redis::logger);
    cpp_redis::subscriber sub;
    std::string st;
    sub.connect("127.0.0.1", 6379, [](const std::string& host, std::size_t port, cpp_redis::subscriber::connect_state status) {
        if (status == cpp_redis::subscriber::connect_state::dropped) {
            std::cout << "client disconnected from " << host << ":" << port << std::endl;
            should_exit.notify_all();
        }
    });
   
    for (int i = 0; i < Symlist.size(); i++) {
        t_positions.push_back(0);
        t_posits.push_back(0.0);
    }
    for (std::string s : pubsubstr) {
        sub.psubscribe(s, [](const std::string& chan, const std::string& msg) { //subscribe to all

        sscanf_s(chan.c_str(), "%[^:]:%[^:]:%[^:]", lsym,10,buf1,10,buf2,10);

        int sympos = indexOf(t_sym, std::string(lsym));
          
        tmst = TMtimestampNow(); //defined here to get type
        datestr = TMint2strdate(tmst / 1000, 'l');
        timestr = TMint2strtime(tmst / 1000, 'l');

            char swlist[100] = "";
            if (sympos<0) { //subscribe to symbols -> then add to table!?  //only for 1st tick of a symbol
                t_sym.push_back(lsym); //out of range?
                sym_idx++;
                sympos = sym_idx-1;
            }
            //if (strncmp(buf1, "trdpb",6) == 0) {
            t_positions[sympos]++;
            maxposit = (maxposit < t_positions[sympos]) ? t_positions[sympos] : maxposit;
            t_posits[sympos]++;
            if (strncmp(buf1, subscr_str.c_str(), 6) == 0) { //trades
                sscanf_s(msg.c_str(), "%[^;];%[^;];%[^;]", ltime,30, lprc, 10,lvol,10);
                int volmln = std::stol(lprc)*std::stol(lvol) / 1e4;
                //int Ltime = (int)std::stoll(ltime)/1000;
                float fprc = std::stof(lprc);
                float fvol = std::stof(lvol);
                std::string S_time = TMint2strtime((pbflag ? std::stoll(ltime) : tmst) / 1000, LOC_GMT);
                snprintf(swlist, sizeof(swlist), "%3d: %-6s %10s %8s %10s %5d %5d", sympos, lsym, S_time.c_str(), lprc, lvol, volmln, t_positions[sympos]);
                //for forex time does not get updated past 20:00:00 -> use current time!
                if (fprc > 0) {
                    if (sym_idx > t_wlist.size()) {
                        t_wlist.push_back(swlist);
                        Ticks.push_back(ticks(lsym));
                        Ticks.back().min = Ticks.back().min = fprc;
                        Ticks.back().addtick(S_time, fprc, fvol);

                        positsize++;
                    }
                    else {
                        t_wlist[sympos] = swlist;
                        //if (fprc > 0) 
                        Ticks[sympos].addtick(S_time,fprc, fvol);
                    }
                }
                //symsel_prc.push_back(atof(prc));
                //if(symsel==sympos) prc_float[prc_float_pos++] = atof(prc);
        
            }
            else if (strcmp(buf1, "bid") == 0)
                sscanf_s(msg.c_str(), "%[^;];%[^;]", ltime,30, lbid,10);
            else if (strcmp(buf1, "ask") == 0)
                sscanf_s(msg.c_str(), "%[^;];%[^;]", ltime,30, lask,10);

        });
    }
    
    //    sscanf_s(chan.c_str(), "%[^:]:%[^:]:%[^:]", lsym,10,buf1,10,buf2,10);
    //    
    //    unsigned sympos=indexOf(t_sym, std::string(lsym));
    //     
    //    char swlist[100] = "";
    //    if (sympos<0) { //subscribe to symbols -> then add to table!?
    //        t_sym.push_back(lsym); //out of range?
    //    }
    //    if (strncmp(buf1, "trades",6) == 0) {
    //        sscanf_s(msg.c_str(), "%[^;];%[^;];%[^;]", ltime,30, lprc, 10,lvol,10);
    //        snprintf(swlist, sizeof(swlist),"%3d: %23s %-7s %8s %10s", sympos, TMint2strdatetime(tmst,'l').c_str(), lsym, lprc, lvol);
    //        //for forex time does not get updated past 20:00:00 -> use current time!
    //        if (sympos < t_wlist.size())
    //            t_wlist[sympos] = swlist;
    //        //symsel_prc.push_back(atof(prc));
    //        //if(symsel==sympos) prc_float[prc_float_pos++] = atof(prc);
    //
    //    }
    //    else if (std::string(buf1)== "bid")
    //        sscanf_s(msg.c_str(), "%[^;];%[^;]", ltime,30, lbid,10);
    //    else if (std::string(buf1)== "ask")
    //        sscanf_s(msg.c_str(), "%[^;];%[^;]", ltime,30, lask,10);
    //    else if (std::string(lsym)=="bar") {
    //        t_barlist.push_back(msg.c_str());
    //        t_splitstr = split(msg, ';');  //split(std::string, )
    //        curbar1min = std::stoi(t_splitstr[0]);
    //        if (curbar1min > lastbar1min) { //1st bar enters automatically
    //            //b1min.push_back(*new Bar(stoi(t_splitstr[0]), stoi(t_splitstr[1]), stod(t_splitstr[2]), stod(t_splitstr[3]), stod(t_splitstr[4]), stod(t_splitstr[5]), stod(t_splitstr[6])));
    //            //b0min.push_back(b2min);
    //            //b2min.set(stoi(t_splitstr[0]), stoi(t_splitstr[1]), stod(t_splitstr[2]), stod(t_splitstr[3]), stod(t_splitstr[4]), stod(t_splitstr[5]), stod(t_splitstr[6]));
    //            barcount++;
    //        }
    //        else {
    //            //b2min.set(stoi(t_splitstr[0]), stoi(t_splitstr[1]), stod(t_splitstr[2]), stod(t_splitstr[3]), stod(t_splitstr[4]), stod(t_splitstr[5]), stod(t_splitstr[6]));
    //        }
    //        if (std::stod(t_splitstr[3]) > maxb) maxb = stod(t_splitstr[3]);
    //        if (stod(t_splitstr[3]) < minb) minb = stod(t_splitstr[4]);
    //        lastbar1min = curbar1min;
    //    }
        
        //sprintf(substr1, "%s - %s",chan.c_str(),msg.c_str());
        //bid should be global to have state! - use tmst not ltime(up to 20:00:00 only)
        //sprintf(substr1, "%23s %-7s %8s %10s %8s %8s", int2strlocaldatetime(tmst), lsym, lprc, lvol, lbid, lask);
     //   snprintf(substr1, sizeof(substr1), "%23s %-7s %8s %10s", TMint2strdatetime(tmst,'l').c_str(), lsym, lprc, lvol);
     //   snprintf(siz, sizeof(siz),"%d:", s_tns2.size());
     //   std::string str1 = substr1;
     //   s_tns2.push_back(siz + str1); //num of trades
     //   std::reverse_copy(s_tns2.end() - (s_tns2.size() >= 1000 ? 1000 : s_tns2.size()), s_tns2.end(), r_tns.begin()); //exception?
                                                                                                                       //no flicker -> memcpy will be faster!? //avoid resize and reserve in the beginning?
                                                                                                                       //}
    sub.commit();

    signal(SIGINT, &sigint_handler);  //necessary or the thread exits!
    std::mutex mtx;
    std::unique_lock<std::mutex> l(mtx);
    should_exit.wait(l);

}

// Data
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//FreeConsole(); //no console window 
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            ImGui_ImplDX9_InvalidateDeviceObjects();
            g_d3dpp.BackBufferWidth  = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
            HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
            if (hr == D3DERR_INVALIDCALL)
                IM_ASSERT(0);
            ImGui_ImplDX9_CreateDeviceObjects();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

int main(int, char**)
{
    tmst = TMtimestampNow(); //defined here to get type
    datestr= TMint2strdate(tmst / 1000,'l');
    timestr = TMint2strtime(tmst / 1000, 'l');

    // Create application window
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("Watchlist"), NULL };
    RegisterClassEx(&wc);
//    HWND hwnd = CreateWindow(_T("Watchlist"), _T("Watchlist"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);
//todo memory for window too...
    hwnd = CreateWindow(_T("Watchlist"), _T("Watchlist"), WS_OVERLAPPEDWINDOW, 50, 100, 1300, 600, NULL, NULL, wc.hInstance, NULL);
    hwnd2 = GetConsoleWindow();
    ShowWindow(hwnd2, SW_HIDE);
    //FreeConsole(); //no console window 

    //hIcon = LoadImage(0, _T("wlist.ico"), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE); //from file
    hIcon = LoadImage(wc.hInstance, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_SHARED | LR_DEFAULTSIZE); //from resource
    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

    std::wstring str1=LoadStringW(102); //read unicode
    std::string str2(str1.begin(), str1.end()); //convert to ascii

    //SendMessage(hwnd2, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    //SendMessage(hwnd2, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

    // Initialize Direct3D
    LPDIRECT3D9 pD3D;
    if ((pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
    {
        UnregisterClass(_T("Watchlist"), wc.hInstance);
        return 0;
    }
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE; // Present with vsync
    //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; // Present without vsync, maximum unthrottled framerate

    // Create the D3DDevice
    if (pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
    {
        pD3D->Release();
        UnregisterClass(_T("Watchlist"), wc.hInstance);
        return 0;
    }

    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    ImGui_ImplDX9_Init(hwnd, g_pd3dDevice);

    // Setup style
    //ImGui::StyleColorsDark();
    ImGui::StyleColorsClassic();

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them. 
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple. 
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'misc/fonts/README.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
//	ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\LiberationMono-Regular.ttf", 16.0f);
//	ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\cour.ttf", 16.0f);
	//IM_ASSERT(font != NULL);
	
	static bool no_titlebar = false;
	static bool no_scrollbar = false;
	static bool no_menu = false;
	static bool no_move = false;
	static bool no_resize = false;
	static bool no_collapse = false;
	static bool no_close = false;
	static bool no_nav = false;
	ImGuiWindowFlags window_flags = 0;
	if (no_titlebar)  window_flags |= ImGuiWindowFlags_NoTitleBar;
	if (no_scrollbar) window_flags |= ImGuiWindowFlags_NoScrollbar;
	if (!no_menu)     window_flags |= ImGuiWindowFlags_MenuBar;
	if (no_move)      window_flags |= ImGuiWindowFlags_NoMove;
	if (no_resize)    window_flags |= ImGuiWindowFlags_NoResize;
	if (no_collapse)  window_flags |= ImGuiWindowFlags_NoCollapse;
	if (no_nav)       window_flags |= ImGuiWindowFlags_NoNav;
	//if (no_close)     p_open = NULL; // Don't pass our bool* to Begin

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    //////////////////////////////
    int cursel = 0;
    char * listitems[] = { "test1", "test2", "test3" };
    std::vector<std::string> items2({ "test1", "test2", "test3" });
    items2.push_back("michalis");
    //float arr[] = { 2,5,12,4,7,34,2 };
    std::vector<float> arr({ 2,5,12,4,7,34,2 });
    std::vector<float> positions(100);


    //https://www.gamedev.net/forums/topic/573496-c-vector-string-to-const-char/
    std::vector<const char*> starts;
    std::transform(items2.begin(), items2.end(), std::back_inserter(starts), std::mem_fn(&std::string::c_str));
    const char** items3 = &starts.front();

    std::vector<std::string> items4({ "test1", "\0","test2","\0", "test3" });
    std::string s1;
s1 = std::accumulate(std::begin(items4), std::end(items4), s1);
char * items5 = "test1\0test2\0test3";

//doesn't work?
//std::stringstream  s3;
//std::copy(items2.begin(), items2.end(), std::ostream_iterator<std::string>(s3, '\0'));
//std::string s4 = s3.str();

//https://stackoverflow.com/questions/9277906/stdvector-to-string-with-custom-delimiter
std::string s2 = std::accumulate(std::begin(items2), std::end(items2), std::string(),
    [](std::string &ss, std::string &s)
{
    return ss.empty() ? s : ss + '\0' + s;
});
s2 += '\0';

//pubsub strings
//Symlist = { "AAPL","TSLA","AMZN" };
Symlist = { "AAPL","NFLX","FB","VZ","TSLA","PYPL","BAC","AMZN","MSFT","GILD","GOOGL","GOOG","C","XOM","T","JPM","IBM","CMCSA","CSCO","PFE","MU","WFC","GE","JNJ","GM","CVX","HPQ","QCOM","INTC","V","ORCL","KO","PG","MRK","EBAY","SLB","DIS","BA","CAT","CELG","GS","TWTR","MCD","APC","HD","WMT","PEP","EUR","GBP","BUD","UTX","AMGN","UPS","MMM","AXP","COP","CL","SPG","NKE","F","DUK","BABA","SBUX","CINF","CTAS","CTXS","CLX","CME","STZ","GLW","COST","CCI","CSX","CMI","DHI","DHR","DRI","DVA","DE","DLPH","DAL","XRAY","DVN","DO","TRIP","WDC","WU","WY","WHR","LOW","LYB","MTB","MAC","MNK","MRO","NEM","NFX","NWL","NTAP","ORLY" };

///////////////////////Initialize
m[ph::pre] = "pre";
m[ph::rth] = "rth";
m[ph::post] = "post";
m[ph::closed] = "closed";

Timers[0] = 1000; //1 sec
Timers[1] = 15000; //15 sec
timestarted = TMtimestampNow(); // when was app started for the 1st time, not timers!
timestartedstr = TMint2strdatetime(timestarted / 1000, 'l');
tday = TMgetdayofweek('l');
//calcphase();


handle = khp("localhost", kdbport);
if (handle < 0) {
    std::cout << "Cannot connect to kdb port " << kdbport << '\n';
    exit(1);
}
handle_pb = khp("localhost", kdbport_pb);
if (handle_pb < 0) {
    std::cout << "Cannot connect to kdb port " << kdbport_pb << '\n';
    exit(1);
}


//startup(); //start on button click
//std::thread t2(&task2, "");  //start redis listener 
//t2.detach();

////////////////////////////
// Main loop
MSG msg;
ZeroMemory(&msg, sizeof(msg));
ShowWindow(hwnd, SW_SHOWDEFAULT);
UpdateWindow(hwnd);

//Compinfo Comp;
//Compdescr Compdes; //should create associations from the start!
//Column Col;

while (msg.message != WM_QUIT)
{
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        continue;
    }
    ImGui_ImplDX9_NewFrame();
    { //cannot have less frames than 60! -> flicker, not usable
        if (sub_start) {
            sub_start = false;
            //if (pbflag) {

            //pbflag = true;
            subscr_str = pbflag ? "trdpb" : "trades"; //pb or rt
            subscr_str2 = pbflag ? "qtspb" : "quotes"; //pb or rt
            for (std::string s : Symlist) {
                pubsubstr.push_back(s + ":crs:*");
                pubsubstr.push_back(s + ":" + subscr_str);
                pubsubstr.push_back(s + ":" + subscr_str2);
                //if (contflag) pubsubstr.push_back(s + ":trdpb");
            }

            std::thread t1(&task1, "");  //start timer thread
            t1.detach();
            //}
        }

        if (show_compinfo) {
            ImGui::Begin("Company Info", &show_compinfo, window_flags);
            if (ImGui::Button("show company info")) {
                show_comp = !show_comp;
            }
            if(show_comp){
                ImGui::InputText("symbol", sym2plot, IM_ARRAYSIZE(sym2info));
                Sym2info = sym2plot;
                std::string qry = "select from (get `:company) where Ticker=`" + Sym2info;
                if (Sym2info != lastSym2info) {
                    ret_getCompInfo = getCompInfo(qry);
                }
                if (ret_getCompInfo != -1) {
                    ImGui::Text("symbol: %s", CompInfo.symbol.c_str());
                    ImGui::Separator();
                    ImGui::Text("name: %s", CompInfo.name.c_str());
                    ImGui::Text("sector: %s", CompInfo.sector.c_str());
                    ImGui::Text("industry: %s", CompInfo.industry.c_str());
                    ImGui::Text("country: %s", CompInfo.country.c_str());
                    if (ImGui::Button("show company description")) {
                        show_descr = !show_descr;
                    }
                    if (ImGui::Button("show company data")) {
                        show_comp_data = !show_comp_data;
                    }                
                }
                lastSym2info = Sym2info;
               // int idx = indexOf(Comp.symbol, Sym2info);
               // if (idx != -1) {
               //     ImGui::Text("symbol: %s", Comp.symbol[idx].c_str());
               //     ImGui::Separator();
               //     ImGui::Text("name: %s", Comp.name[idx].c_str());
               //     ImGui::Text("sector: %s", Comp.sector[idx].c_str());
               //     ImGui::Text("industry: %s", Comp.industry[idx].c_str());
               //     ImGui::Text("country: %s", Comp.country[idx].c_str());
               //     if (ImGui::Button("show company description")) {
               //         show_descr =! show_descr;
               //     }
               //     if (ImGui::Button("show company data")) {
               //         show_comp_data = !show_comp_data;
               //     }
               // }
            }
            ImGui::End();
        }
        
        if (show_descr) {
            ImGui::Begin("Company Description", &show_descr, window_flags);
            std::string Sym2info = sym2plot;
            std::string qry = "select from get `:companyinfo where symbol = `" + Sym2info;
            if (Sym2info != lastSym2descr) {
                ret_getCompDescr = getCompDescr(qry);
            }
            if (ret_getCompDescr != -1) {
                ImGui::TextWrapped("%s", CompDescr.c_str());
            }
            lastSym2descr = Sym2info;
            ImGui::End();
        }
        
        if (show_comp_data) {
            ImGui::Begin("Company Data", &show_comp_data, window_flags);
            std::string Sym2info = sym2plot;
            std::string qry = "0!select by sym from (0!fdata) where sym=`" + Sym2info;
            if (Sym2info != lastSym2fdata) {
                ret_getFData = getFData(qry);
            }
            if (ret_getFData != -1) {
              for (int j = 0; j < FData.size(); j++){
                    ImGui::TextWrapped("%s : %s", FNames[j].c_str(), FData[j].c_str());
                    //ImGui::SameLine();
              }
            }
            lastSym2fdata = Sym2info;
            ImGui::End();
        }

        if (show_tns_window) { //crashes if trying to plot too many tns lines (vsprintf exception, or vector out of bounds at ImGui::Text)
            ImGui::Begin("T&S", &show_plot_window, window_flags);
            if (ImGui::Button("show t&s")) {
                show_tns = !show_tns;
            }; ImGui::SameLine(); ImGui::InputInt("list size", &maxtns);
            if (show_tns) {
                ImGui::InputText("symbol", sym2plot, IM_ARRAYSIZE(sym2plot));
                //Sym2plot = std::string(sym2plot);
                //std::transform(Sym2plot.begin(), Sym2plot.end(), Sym2plot.begin(), ::toupper);
                //sidx = indexOf(t_sym, Sym2plot);  //exception here!
                sidx = indexOf(t_sym, std::string(sym2plot));
                if (sidx >= 0) {
                    maxsizetns = ((Ticks[sidx].t_dvol.size() > maxtns) ? (Ticks[sidx].t_dvol.size() - maxtns) : 0);
                    ImGui::PushItemWidth(400);
                    if (((std::string(sym2plot) == "EUR") || std::string(sym2plot) == "GBP"))
                    //if ((Sym2plot == "EUR") || (Sym2plot == "GBP"))
                            fmt_wl = fmt_cash;
                    else
                        fmt_wl = fmt_stk;
                    
                    ImGui::Text("  idx       time   price volume");
                    for (int i = Ticks[sidx].t_dvol.size() - 1; i > maxsizetns - 1; i--) {
                        
                        //ImGui::Text("%5d %10s %7.2f %6.0f", i, Ticks[sidx].t_time[i].c_str(),
                        ImGui::Text(fmt_wl.c_str(), i, Ticks[sidx].t_time[i].c_str(),
                            Ticks[sidx].t_prc[i], Ticks[sidx].t_dvol[i]);
                    }
                       //for (int i = Ticks[sidx].t_dvol.size()-1; i >-1 ; i--) {
                           //ImGui::Text("%5d %10s %7.2f %6.0f", i, TMint2strtime(Ticks[sidx].t_time[i] / 1000, LOC_GMT).c_str(),
                           //    Ticks[sidx].t_prc[i], Ticks[sidx].t_dvol[i]);
                       //for (int i = 0; i< Ticks[sidx].t_dvol.size(); i++) {
                       //    sprintf_s(strbuf, sizeof(strbuf), "%5d %10s %7.2f %6.0f", i, Ticks[sidx].t_time[i].c_str(),
                       //        Ticks[sidx].t_prc[i], Ticks[sidx].t_dvol[i]);
                       //    ImGui::Text("%s", strbuf);
                       //    //ImGui::Text("%5d %10s %7.2f %6.0f", i, Ticks[sidx].t_time[i].c_str(),
                       //    //                                                 Ticks[sidx].t_prc[i], Ticks[sidx].t_dvol[i]);
                       //}
                       ImGui::PopItemWidth();
                   }
               }
               ImGui::End();
           }

           if (show_plot_window) {
               ImGui::Begin("Plot", &show_plot_window, window_flags);
               if (ImGui::Button("show chart")) {
                   show_plot = !show_plot;
               }; ImGui::SameLine();  ImGui::Text("num of symbols: %d", Ticks.size());
               if (show_plot) {
                   //ImGui::Text("symbol: %s", Sym2plot.c_str());
                   ImGui::InputText("symbol", sym2plot, IM_ARRAYSIZE(sym2plot));
                   //sidx = indexOf(t_sym, std::string(sym2plot));
                   if (sidx >= 0) {
                       ImGui::PushItemWidth(400);
                       ImGui::PlotLines("price", Ticks[sidx].t_prc.data(), Ticks[sidx].t_prc.size(), 0, NULL, Ticks[sidx].min, Ticks[sidx].max, ImVec2(0, 100));
                       //ImGui::PlotHistogram("volume", Ticks[sidx].t_volume.data(), Ticks[sidx].t_volume.size(), 0, NULL, 0.0f, Ticks[sidx].t_volume.back(), ImVec2(0, 100));
                       ImGui::PopItemWidth();
                   }
               }
               ImGui::End();
           }

          if (show_settings_window) {
            ImGui::Begin("Settings", &show_settings_window,window_flags);
            if(!timers_set)
                ImGui::Checkbox("playback mode", &pbflag);
            ImGui::Checkbox("enable console", &show_console);
            //ImGui::Text("current time: %s %s", datestr.c_str(), timestr.c_str());
            if (show_console) ShowWindow(hwnd2, SW_SHOW);
            else ShowWindow(hwnd2, SW_HIDE);

            if (pbflag) ImGui::TextColored(color_blue, "Playback Mode");
            else ImGui::TextColored(color_green, "Live Mode");
            ImGui::PushItemWidth(250);
            ImGui::InputText("kdb host", kdbhost,IM_ARRAYSIZE(kdbhost));
            //ImGui::InputText("kdb host", (char *)kdbhost.c_str(), 30); // IM_ARRAYSIZE(kdbhost)); //crashes
            ImGui::InputInt("kdb port", &kdbport, false);
            ImGui::InputInt("kdb port(playback)", &kdbport_pb, false);
            ImGui::Text("last kdb timestamp : %s since %d secs", kdbtsstr.c_str(),now2kdbts);
            ImGui::Text("app was started on: %s", timestartedstr.c_str());
            ImGui::Text("days running : %d", daysrunning);
            ImGui::Text("current phase : %s", m[phase].c_str());
            ImGui::Text("date: %s time : %s olddate: %s day: %s", datestr.c_str(),timestr.c_str(),olddatestr.c_str(),weekdays[tday]);

            ImGui::PopItemWidth();
            if (ImGui::Button("start timers", ImVec2(250, 20))) { //is executed only once!
                sub_start = true;
            }
            
          
            //if (A.size() > 0) {
            //    for (AData a : A) {
            //        ImGui::Text("a.m_sid: %d | a.m_bid: %d", a.m_sid, a.m_bid);
            //    }
            //}

            //if (ImGui::Button("Close", ImVec2(250, 20))) {
            //  show_settings_window = false;
            //}
            ImGui::End();
          }
          if (show_init_window)
          {
              ImGui::Begin("Watchlist", &show_init_window);
              ImGui::TextColored(color_magenta, "Times & Prices");
              ImGui::PushItemWidth(250);
              ImGui::PopItemWidth();

              // ImGui::ListBox("mylist", &cursel, listitems, 3);// sizeof(listitems));
          //    ImGui::ListBox("mylist", &cursel, items3, items2.size());
              //ImGui::PlotHistogram ("histo", arr, IM_ARRAYSIZE(arr), 0, NULL, 0.0f, 70.0f, ImVec2(0, 100));
              //ImGui::PlotHistogram("histo", arr.data(), arr.size(), 0, NULL, 0.0f, 70.0f, ImVec2(0, 100));
              ImGui::PlotHistogram("num of trades", t_posits.data(), positsize, 0, NULL, 0.0f, maxposit, ImVec2(0, 100));
              //ImGui::Combo("combo", &cursel, (const char *)items4.data());
              //ImGui::Combo("combo", &cursel, (const char *) items5);
          //    ImGui::Combo("combo", &cursel, (const char *)s2.c_str());
              //ImGui::Combo("combo", &cursel, (const char *)std::string("ert\0dfg\0ert").data());
              ImGui::PushItemWidth(500);
              ImGui::Text(" idx symbol       time    price     volume   mln$   ntr");
              if (ImGui::ListBox("wlist", &list_sel, t_wlist, t_wlist.size())) {
                      sidx = ((list_sel == -1) ? 0 : list_sel);
                      snprintf(sym2plot, sizeof(sym2plot), "%s", t_sym[sidx].c_str());
                  
              }
              ImGui::PopItemWidth();
              ImGui::End();
          }
		}
		
		////////////////END

        // Rendering
        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, false);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x*255.0f), (int)(clear_color.y*255.0f), (int)(clear_color.z*255.0f), (int)(clear_color.w*255.0f));
        g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);//draw background
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

        // Handle loss of D3D9 device
        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
        {
            ImGui_ImplDX9_InvalidateDeviceObjects();
            g_pd3dDevice->Reset(&g_d3dpp);
            ImGui_ImplDX9_CreateDeviceObjects();
        }
    }

    ImGui_ImplDX9_Shutdown();
    ImGui::DestroyContext();

    if (g_pd3dDevice) g_pd3dDevice->Release();
    if (pD3D) pD3D->Release();
    DestroyWindow(hwnd);
    UnregisterClass(_T("Watchlist"), wc.hInstance);

    return 0;
}



/*todo
    - skip frames - use count mod %
    - idx in watchlist mutliple entries -> empty list on startup!... - different value -> jump in price! todo fix


    */
