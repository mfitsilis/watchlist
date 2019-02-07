#define ST std::string
#define SN std::to_string
#define SP ST(" ")
#define SM ST(";")
#define SBT ST(";`")
#define SB(x) ST(";`"+x)
#define SC ST(")")
#define CR ST("\n")

///// Global variables

typedef long long Long;
Long tmst; //msec precision
Long tmst_nextmin;
int tmst_sec=0;

Long timestart;
Long target;
Long timestarted;

HWND hwnd;
HWND hwnd2;
HANDLE hIcon;
HANDLE hIcon2;

long tspre;     // 09:27:00
long tscrsop;   // 09:30:10
long tssod;     // 09:30:00
long tsmoc;     // 15:49:00

//std::vector<int> Timers;
//std::vector<Long> TimersTarget;
//std::vector<int> TimersStatus;
int Timers[10] = { 0 };
Long TimersTarget[10] = { 0 };
int TimersStatus[10] = { 0 };
bool timer_status = false;
bool timers_set = false;
bool sub_start = false;
bool show_console = false;

int  kdbport = 5500;
int  kdbport_pb = 5300; //for playback

bool contflag = false;
bool testflag = false;
bool algosbusy = false;

bool finished_thread = true;

int restarted = 0; //after cont and finishing playback of tickdata set to 1
int daysrunning = 0;

int marketopen = 0;
int xth = 0;
int eodflag = 0;
Long tseod1; //for RT

const char * phasestr[] = { "pre", "rth", "post", "closed" };
enum class ph { pre, rth, post, closed };
std::map<ph, std::string> m;
ph phase;

int kdbts; //timestamp of last kdb time
Long now2kdbts;

//int num1=0;
int nuc=0; //1min bars til sod, is <0 pre sod, >0 after

int handle;
int handle_pb;

bool pbflag = true;// false;
int tday = 0;
int tmonth;
int tmday;
int tilnexthol;
bool todayhol;
const int wknd = 7;

Long timeold=0;

std::string subscr_str = "";
std::string subscr_str2 = "";
int tot_mins = 390;
int eod_mins = 960;
const char poststr[] = "20:00:00";
char eodstr[] = "16:00:00";
const char eodnstr[] = "16:00:00";
char mocstr[] = "15:49:00";
const char mocnstr[] = "15:49:00";
const char eodestr[] = "13:00:00"; //eod early
const char mocestr[] = "12:49:00"; //moc early
const char revstr[] = "09:37:59";
const char crsopstr[] = "09:30:10";
const char sodstr[] = "09:30:00";
const char opgstr[] = "09:27:00";
const char prestr[] = "04:00:00";

std::string timestartedstr;
char kdbhost[40]="localhost";
//std::string kdbhost("localhost");
std::string kdbtsstr = "";
std::string datetimestr;
std::string timestr; // HH:MM:SS
std::string datestr;// YYYY.mm.dd
std::string olddatestr;
std::string datestr_start;

const char * wdays[]={"Sat","Sun","Mon","Tue","Wed","Thu","Fri"};
const char * weekdays[] = { "Saturday","Sunday","Monday","Tuesday","Wednesday","Thursday","Friday" };


//global vectors
std::vector<std::string>Symlist; //all unique syms
std::vector<std::string>Pubsubstr;

double hi, lo, cl, op, vo, med, rmed; //static because algos are not executed in parallel
double hi1, lo1, cl1, op1, vo1, med1, rmed1;
double hi2, lo2, cl2, op2, vo2, med2, rmed2;

std::vector<int> validlivelist{ 0, 1, 2 };
std::vector<int> validsteplist{ 1, 2, 3, 5, 10, 15, 30 };
std::vector<std::string> validseclist{ "STK", "CASH" };

/////////////////////////////// widget values

ImColor color_black = ImColor(0, 0, 0);
ImColor color_red = ImColor(255, 0, 0);
ImColor color_green = ImColor(0, 255, 0);
ImColor color_yellow = ImColor(255, 255, 0);
ImColor color_white = ImColor(255, 255, 255);
ImColor color_cyan = ImColor(0, 255, 255);
ImColor color_blue = ImColor(0, 0, 255);
ImColor color_magenta = ImColor(255, 0, 255);

bool show_settings_window = true;
bool show_init_window = true;
bool show_plot_window = true;
bool show_plot = true;
bool show_tns_window = true;
bool show_tns = true;
bool show_compinfo = true;
bool show_comp = true;
bool show_descr = false;
bool show_comp_data = false;

char sym2plot[6] = "";
char sym2info[6] = "";
std::string Sym2info="";
std::string lastSym2info=""; //so there is no continuous qry!
std::string lastSym2descr = ""; //so there is no continuous qry!
std::string lastSym2fdata = ""; //so there is no continuous qry!
std::string Sym2plot="";
int ret_getCompInfo = -1;
int ret_getCompDescr = -1;
int ret_getFData = -1;
std::string CompDescr;
std::vector<std::string> FNames;
std::vector<std::string> FData;

std::condition_variable should_exit;

class ticks {
 public:
    std::string sym;
    std::vector<std::string> t_time;
    std::vector<float> t_prc;
    std::vector<float> t_volume;
    std::vector<float> t_dvol;
    float lastprc = 0.0;
    float lastvol = 0.0;
    float min=0.0;
    float max = 0.0;
    float maxv = 0.0;
    float volume = 0.0;
    ticks(std::string sym1) {
        sym = sym1;
    };
    void addtick(std::string time,float prc, float vol) {
        if (prc > max) max = prc;
        if (prc < min) min = prc;
        if (vol - lastvol > maxv) maxv = vol - lastvol;
        t_time.push_back(time);
        t_prc.push_back(prc);
        t_volume.push_back(vol);
        t_dvol.push_back(vol-lastvol);
        lastprc = prc;
        lastvol = vol;
    };
};

float maxposit = 0;
int positsize = 0;
std::vector<int> t_positions;
std::vector<float> t_posits;
std::vector<std::string> t_prc;
std::vector<std::string> t_volume;
//std::vector<std::string> t_time;
std::vector<std::string> t_bid;
std::vector<std::string> t_ask;
std::vector<std::string> t_splitstr;
std::vector<std::string> t_barlist;
std::vector<std::string> s_tns;
std::vector<std::string> s_tns2;
std::vector<std::string> r_tns;
std::vector<std::string> t_wlist;
//std::vector<std::string> t_splitstr;
std::vector<std::string> pubsubstr; //for pubsub to all symbols

struct CompInfo{
    std::string symbol;
    std::string name;
    std::string sector;
    std::string industry;
    std::string country;
    std::string description;
} CompInfo;


std::vector<std::string> t_sym;
int sidx = 0;
std::vector<ticks> Ticks;
int maxsizetns = 20;
int maxtns = 20;

int list_sel = -1; //click on listbox gives idx

char fmt_stk[40] = "%5d %10s %7.2f %6.0f";
char fmt_cash[40] = "%5d %10s %7.5f %6.0f";
std::string fmt_wl = fmt_stk;


int lastbar1min = 0;
int curbar1min = 0;
int lastbar1min_s = 0; //show
int barcount = 0;
int barcount_s = 0;

double minb = 100000.0;
double maxb = 0.0;

char siz[10], lsym[10], ltime[30], lprc[10], lvol[20], lbid[10], lask[10], buf1[10], buf2[10];
char substr1[100];

int sym_idx = 0;
