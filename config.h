/* See LICENSE file for copyright and license details. */
#include "fibonacci.c"

/* appearance */
static unsigned int borderpx		= 1;        /* border pixel of windows */
static unsigned int gappx		= 5;        /* gaps between windows */
static unsigned int snap		= 32;       /* snap pixel */
static const int swallowfloating	= 0;        /* 1 means swallow floating windows by default */
static int showbar			= 1;        /* 0 means no bar */
static int topbar			= 1;        /* 0 means bottom bar */

static char font[]			= "monospace:size=10";
static const char *fonts[]		= { font };
static char dmenuFont[]			= "monospace:size=10";
static char dmenuBgColor[]		= "#222222";
static char dmenuTextColor[]		= "#bbbbbb";
static char dmenuActiveTextColor[]	= "#eeeeee";
static char dmenuActiveBgColor[]	= "#005577";
static char layoutBgColor[]		= "#222222";
static char layoutBorderColor[]		= "#444444";
static char layoutTextColor[]		= "#bbbbbb";
static char statusBgColor[]		= "#222222";
static char statusBorderColor[]		= "#444444";
static char statusTextColor[]		= "#bbbbbb";
static char groupActiveBgColor[]	= "#222222";
static char groupActiveBorderColor[]	= "#444444";
static char groupActiveTextColor[]	= "#bbbbbb";
static char groupBgColor[]		= "#222222";
static char groupBorderColor[]		= "#444444";
static char groupTextColor[]		= "#bbbbbb";
static char tagActiveBgColor[]		= "#005577";
static char tagActiveBorderColor[]	= "#005577";
static char tagActiveTextColor[]	= "#eeeeee";
static char tagBgColor[]		= "#222222";
static char tagBorderColor[]		= "#444444";
static char tagTextColor[]		= "#bbbbbb";
static char winActiveBgColor[]		= "#005577";
static char winActiveBorderColor[]	= "#005577";
static char winActiveTextColor[]	= "#eeeeee";
static char winBgColor[]		= "#222222";
static char winBorderColor[]		= "#444444";
static char winTextColor[]		= "#bbbbbb";

static const unsigned int alphaText = OPAQUE;
static const unsigned int alphaBackground = 0xd0;
static const unsigned int alphaBorder = OPAQUE;

static char *colors[][3]		= {
	[SchemeLayout] = { layoutTextColor, layoutBgColor, layoutBorderColor },
	[SchemeStatus] = { statusTextColor, statusBgColor, statusBorderColor },
	[SchemeGroup] = { groupTextColor, groupBgColor, groupBorderColor },
	[SchemeGroupActive] = { groupActiveTextColor, groupActiveBgColor, groupActiveBorderColor },
	[SchemeTag] = { tagTextColor, tagBgColor, tagBorderColor },
	[SchemeTagActive] = { tagActiveTextColor, tagActiveBgColor, tagActiveBorderColor },
	[SchemeWin] = { winTextColor, winBgColor, winBorderColor },
	[SchemeWinActive] = { winActiveTextColor, winActiveBgColor, winActiveBorderColor },
};

static const unsigned int alphas[][3]      = {
	[SchemeLayout] = { alphaText, alphaBackground, alphaBorder },
	[SchemeLayout] = { alphaText, alphaBackground, alphaBorder },
	[SchemeStatus] = { alphaText, alphaBackground, alphaBorder },
	[SchemeGroup] = { alphaText, alphaBackground, alphaBorder },
	[SchemeGroupActive] = { alphaText, alphaBackground, alphaBorder },
	[SchemeTag] = { alphaText, alphaBackground, alphaBorder },
	[SchemeTagActive] = { alphaText, alphaBackground, alphaBorder },
	[SchemeWin] = { alphaText, alphaBackground, alphaBorder },
	[SchemeWinActive] = { alphaText, alphaBackground, alphaBorder },
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class     instance  title           tags mask  isfloating  isterminal  noswallow  monitor */
	{ "Gimp",    NULL,     NULL,           0,         1,          0,           0,        -1 },
	{ "Firefox", NULL,     NULL,           1 << 8,    0,          0,	   1,        -1 },
	{ "St",      NULL,     NULL,           0,         0,          1,           0,        -1 },
	{ NULL,      NULL,     "Event Tester", 0,         0,          0,           1,        -1 }, /* xev */
};

/* layout(s) */
static float mfact	= 0.55; /* factor of master area size [0.05..0.95] */
static int nmaster	= 1;    /* number of clients in master area */
static int resizehints	= 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */

/* Bartabgroups properties */
static void (*bartabmonfns[])(Monitor *) = { monocle /* , customlayoutfn */ };
static void (*bartabfloatfns[])(Monitor *) = { NULL /* , customlayoutfn */ };

static const Layout layouts[] = {
	/* symbol	arrange function */
	{ "[]=",	tile },    /* first entry is default */
	{ "><>",	NULL },    /* no layout function means floating behavior */
	{ "[M]",	monocle },
	{ "|M|",	centeredmaster },
	{ "[@]",	spiral },
	{ "[\\]",	dwindle },
	{ "[D]",	deck },
	{ "TTT",	bstack },
	{ "===",	bstackhoriz },
};

/* key definitions */
#define MODKEY Mod1Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

#define STATUSBAR "dwmblocks"

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenuFont, "-nb", dmenuBgColor, "-nf", dmenuTextColor, "-sb", dmenuActiveBgColor, "-sf", dmenuActiveTextColor, NULL };

static const char *termcmd[]  = { "st", NULL };

/*
 * Xresources preferences to load at startup
 */
ResourcePref resources[] = {
	{ "font",               	STRING,		&font },
	{ "dmenuFont",			STRING,		&dmenuFont },
	{ "dmenuBgColor",		STRING, 	&dmenuBgColor },
	{ "dmenuTextColor",		STRING, 	&dmenuTextColor },
	{ "dmenuActiveTextColor",	STRING, 	&dmenuActiveTextColor },
	{ "dmenuActiveBgColor",		STRING, 	&dmenuActiveBgColor },
	{ "layoutBgColor",		STRING, 	&layoutBgColor },
	{ "layoutBorderColor",		STRING, 	&layoutBorderColor },
	{ "layoutTextColor",		STRING, 	&layoutTextColor },
	{ "statusBgColor",		STRING, 	&statusBgColor },
	{ "statusBorderColor",		STRING, 	&statusBorderColor },
	{ "statusTextColor",		STRING, 	&statusTextColor },
	{ "groupActiveBgColor",		STRING, 	&groupActiveBgColor },
	{ "groupActiveBorderColor",	STRING, 	&groupActiveBorderColor },
	{ "groupActiveTextColor",	STRING, 	&groupActiveTextColor },
	{ "groupBgColor",		STRING, 	&groupBgColor },
	{ "groupBorderColor",		STRING, 	&groupBorderColor },
	{ "groupTextColor",		STRING, 	&groupTextColor },
	{ "tagActiveBgColor",		STRING, 	&tagActiveBgColor },
	{ "tagActiveBorderColor",	STRING, 	&tagActiveBorderColor },
	{ "tagActiveTextColor",		STRING, 	&tagActiveTextColor },
	{ "tagBgColor",			STRING, 	&tagBgColor },
	{ "tagBorderColor",		STRING, 	&tagBorderColor },
	{ "tagTextColor",		STRING, 	&tagTextColor },
	{ "winActiveBgColor",		STRING, 	&winActiveBgColor },
	{ "winActiveBorderColor",	STRING, 	&winActiveBorderColor },
	{ "winActiveTextColor",		STRING, 	&winActiveTextColor },
	{ "winBgColor",			STRING, 	&winBgColor },
	{ "winBorderColor",		STRING, 	&winBorderColor },
	{ "winTextColor",		STRING, 	&winTextColor },
	{ "borderpx",          		INTEGER, 	&borderpx },
	{ "gappx",          		INTEGER, 	&gappx },
	{ "snap",			INTEGER, 	&snap },
	{ "showbar",          		INTEGER, 	&showbar },
	{ "topbar",          		INTEGER, 	&topbar },
	{ "nmaster",          		INTEGER, 	&nmaster },
	{ "resizehints",       		INTEGER, 	&resizehints },
	{ "mfact",			FLOAT,   	&mfact },
};

static const Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_p,      spawn,          {.v = dmenucmd } },
	{ MODKEY|ShiftMask,             XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY|ShiftMask,		XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_j,      inplacerotate,  {.i = +1} },
	{ MODKEY|ShiftMask,             XK_k,      inplacerotate,  {.i = -1} },
	{ MODKEY|ShiftMask,             XK_h,      inplacerotate,  {.i = +2} },
	{ MODKEY|ShiftMask,             XK_l,      inplacerotate,  {.i = -2} },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY,                       XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY|ShiftMask,             XK_c,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} }, // tile
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} }, // float
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} }, // monocle
	{ MODKEY,                       XK_u,      setlayout,      {.v = &layouts[3]} }, // centeredmaster
	{ MODKEY,                       XK_o,      setlayout,      {.v = &layouts[4]} }, // spiral
	{ MODKEY|ShiftMask,             XK_o,      setlayout,      {.v = &layouts[5]} }, // dwindle
	{ MODKEY,             		XK_y,      setlayout,      {.v = &layouts[6]} }, // deck
	{ MODKEY,             		XK_n,      setlayout,      {.v = &layouts[7]} }, // bstack
	{ MODKEY|ShiftMask,		XK_n,      setlayout,      {.v = &layouts[8]} }, // bstackhoriz
	{ MODKEY,                       XK_space,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
	{ MODKEY,                       XK_minus,  setgaps,        {.i = -1 } },
	{ MODKEY,                       XK_equal,  setgaps,        {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_equal,  setgaps,        {.i = 0  } },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	{ MODKEY|ShiftMask,             XK_r,      quit,           {1} }, /* restart */
	{ MODKEY|ControlMask|ShiftMask, XK_q,      quit,           {0} }, /* quit */
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button1,        sigstatusbar,	{.i = 1 } },
	{ ClkStatusText,        0,              Button2,        sigstatusbar,	{.i = 2 } },
	{ ClkStatusText,        0,              Button3,        sigstatusbar,	{.i = 3 } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

