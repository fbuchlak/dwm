/* See LICENSE file for copyright and license details. */

/* appearance */
static unsigned int borderpx      = 1;  /* border pixel of windows */
static unsigned int gappih        = 10; /* horiz inner gap between windows */
static unsigned int gappiv        = 10; /* vert inner gap between windows */
static unsigned int gappoh        = 10; /* horiz outer gap between windows and screen edge */
static unsigned int gappov        = 10; /* vert outer gap between windows and screen edge */
static int smartgaps              = 0;  /* 1 means no outer gap when there is only one window */
static unsigned int snap          = 32; /* snap pixel */
static const int swallowfloating  = 0;  /* 1 means swallow floating windows by default */
static int showbar                = 1;  /* 0 means no bar */
static int topbar                 = 1;  /* 0 means bottom bar */

static char font[]                = "monospace:size=10";
static const char *fonts[]        = { font };

static char bgColor[]       = "#222222"; // background color 
static char boColor[]       = "#222222"; // border color
static char fgColor[]       = "#bbbbbb"; // text color

static char bgColorActive[] = "#005577"; // background color 
static char boColorActive[] = "#005577"; // border color
static char fgColorActive[] = "#eeeeee"; // text color

static const unsigned int alphaBg     = 0xd0;
static const unsigned int alphaBorder = OPAQUE;
static const unsigned int alphaFg     = OPAQUE;

static char *colors[][3] = {
    [SchemeNorm] = { fgColor,       bgColor,       boColor },
    [SchemeSel] =  { fgColorActive, bgColorActive, boColorActive },
};

static const unsigned int alphas[][3] = {
    [SchemeNorm] = { alphaFg, alphaBg, alphaBorder },
    [SchemeSel] =  { alphaFg, alphaBg, alphaBorder },
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
    /* xprop(1):
     *  WM_CLASS(STRING) = instance, class
     *  WM_NAME(STRING) = title
     */
    /* class     instance  title           tags mask  isfloating  isterminal  noswallow  monitor */
    { "Gimp",    NULL,     NULL,           0,         1,          0,           0,        -1 },
    { "Firefox", NULL,     NULL,           1 << 8,    0,          0,           1,        -1 },
    { "St",      NULL,     NULL,           0,         0,          1,           0,        -1 },
    { NULL,      NULL,     "Event Tester", 0,         0,          0,           1,        -1 }, /* xev */
};

/* layout(s) */
static float mfact              = 0.6;  /* factor of master area size [0.05..0.95] */
static int nmaster              = 1;    /* number of clients in master area */
static int resizehints          = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1;    /* 1 will force focus on the fullscreen window */

/* Bartabgroups properties */
static void (*bartabmonfns[])(Monitor *)   = { monocle /* , customlayoutfn */ };
static void (*bartabfloatfns[])(Monitor *) = { NULL    /* , customlayoutfn */ };

//#define FORCE_VSPLIT 1  /* nrowgrid layout: force two clients to always split vertically */
#include "vanitygaps.c"

/* first entry is default */
static const Layout layouts[] = {
    /* symbol arrange function */
    { "[]=",  tile            },
    { "><>",  NULL            }, // floating
    { "[M]",  monocle         },
    { "|M|",  centeredmaster  },
    { "[@]",  spiral          },
    { "[\\]", dwindle         },
    { "[D]",  deck            },
    { "TTT",  bstack          },
    { "===",  bstackhoriz     },
};

/* key definitions */
#define MODKEY Mod1Mask
#define TAGKEYS(KEY,TAG) \
    { MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
    { MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
    { MODKEY|ControlMask,           KEY,      focusnthmon,    {.i  = TAG }     }, \
    { MODKEY|ControlMask|ShiftMask, KEY,      tagnthmon,      {.i  = TAG }     },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

#define STATUSBAR "dwmblocks"

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", font, "-nb", bgColor, "-nf", fgColor, "-sb", bgColorActive, "-sf", fgColorActive, NULL };

static const char *termcmd[]  = { "st", NULL };

static const char *lockcmd[]  = { "slock", NULL };

/* Xresources preferences to load at startup */
ResourcePref resources[] = {
    { "font",           STRING,   &font           },
    { "bgColor",        STRING,   &bgColor        },
    { "boColor",        STRING,   &boColor        },
    { "fgColor",        STRING,   &fgColor        },
    { "bgColorActive",  STRING,   &bgColorActive  },
    { "boColorActive",  STRING,   &boColorActive  },
    { "fgColorActive",  STRING,   &fgColorActive  },
    { "borderpx",       INTEGER,  &borderpx       },
    { "gappih",         INTEGER,  &gappih         },
    { "gappiv",         INTEGER,  &gappiv         },
    { "gappoh",         INTEGER,  &gappoh         },
    { "gappov",         INTEGER,  &gappov         },
    { "snap",           INTEGER,  &snap           },
    { "showbar",        INTEGER,  &showbar        },
    { "topbar",         INTEGER,  &topbar         },
    { "nmaster",        INTEGER,  &nmaster        },
    { "resizehints",    INTEGER,  &resizehints    },
    { "mfact",          FLOAT,    &mfact          },
};

static const Key keys[] = {
    /*--- Layouts ------------------------------------------------------------------*/
    /* modifier                     key        function        argument             */
    { MODKEY,                       XK_t,      setlayout,      { .v = &layouts[0] } }, // tile
    { MODKEY,                       XK_f,      setlayout,      { .v = &layouts[1] } }, // float
    { MODKEY,                       XK_m,      setlayout,      { .v = &layouts[2] } }, // monocle
    { MODKEY,                       XK_u,      setlayout,      { .v = &layouts[3] } }, // centeredmaster
    { MODKEY,                       XK_o,      setlayout,      { .v = &layouts[4] } }, // spiral
    { MODKEY|ShiftMask,             XK_o,      setlayout,      { .v = &layouts[5] } }, // dwindle
    { MODKEY,                       XK_y,      setlayout,      { .v = &layouts[6] } }, // deck
    { MODKEY,                       XK_i,      setlayout,      { .v = &layouts[7] } }, // bstack
    { MODKEY|ShiftMask,             XK_i,      setlayout,      { .v = &layouts[8] } }, // bstackhoriz
    { MODKEY,                       XK_space,  togglefloating, { 0 }                }, 
    /*--- Execute ------------------------------------------------------------------*/
    /* modifier                     key        function        argument             */
    { MODKEY,                       XK_p,      spawn,          { .v = dmenucmd }    },
    { MODKEY|ShiftMask,             XK_Return, spawn,          { .v = termcmd }     },
    /*--- Stack --------------------------------------------------------------------*/
    /* modifier                     key        function        argument             */
    { MODKEY,                       XK_Return, zoom,           { 0 }                },
    { MODKEY,                       XK_j,      focusstack,     { .i = +1 }          },
    { MODKEY,                       XK_k,      focusstack,     { .i = -1 }          },
    { MODKEY,                       XK_h,      setmfact,       { .f = -0.05 }       },
    { MODKEY,                       XK_l,      setmfact,       { .f = +0.05 }       },
    { MODKEY|ShiftMask,             XK_j,      inplacerotate,  { .i = +1 }          }, // rotate master/stack
    { MODKEY|ShiftMask,             XK_k,      inplacerotate,  { .i = -1 }          }, // rotate master/stack
    { MODKEY|ShiftMask,             XK_h,      inplacerotate,  { .i = +2 }          }, // rotate all 
    { MODKEY|ShiftMask,             XK_l,      inplacerotate,  { .i = -2 }          }, // rotate all
    { MODKEY|ControlMask|ShiftMask, XK_j,      incnmaster,     { .i = -1 }          },
    { MODKEY|ControlMask|ShiftMask, XK_k,      incnmaster,     { .i = +1 }          },
    /*--- Mons ---------------------------------------------------------------------*/
    /* modifier                     key        function        argument             */
    { MODKEY,                       XK_comma,  focusmon,       { .i = -1 }          },
    { MODKEY,                       XK_period, focusmon,       { .i = +1 }          },
    { MODKEY|ShiftMask,             XK_comma,  tagmon,         { .i = -1 }          },
    { MODKEY|ShiftMask,             XK_period, tagmon,         { .i = +1 }          },
    /*--- Gaps ---------------------------------------------------------------------*/
    /* modifier                     key        function        argument             */
    { MODKEY,                       XK_Down,   incrgaps,       { .i = -1 }          },
    { MODKEY,                       XK_Up,     incrgaps,       { .i = +1 }          },
    { MODKEY|ShiftMask,             XK_Down,   incrgaps,       { .i = -5 }          },
    { MODKEY|ShiftMask,             XK_Up,     incrgaps,       { .i = +5 }          },
    { MODKEY|ControlMask|ShiftMask, XK_Down,   incrgaps,       { .i = -15 }         },
    { MODKEY|ControlMask|ShiftMask, XK_Up,     incrgaps,       { .i = +15 }         },
    { MODKEY,                       XK_equal,  defaultgaps,    { 0 }                },
    { MODKEY|ShiftMask,             XK_equal,  togglegaps,     { 0 }                },
    /*--- Misc ---------------------------------------------------------------------*/
    /* modifier                     key        function        argument             */
    { MODKEY,                       XK_Tab,    view,           { 0 }                },
    { MODKEY|ShiftMask,             XK_b,      togglebar,      { 0 }                },
    { MODKEY,                       XK_0,      view,           { .ui = ~0 }         },
    { MODKEY|ShiftMask,             XK_0,      tag,            { .ui = ~0 }         },
    /*--- Tags ---------------------------------------------------------------------*/
    /* modifier                     key        function        argument             */
    TAGKEYS(                        XK_1,                      0                    )
    TAGKEYS(                        XK_2,                      1                    )
    TAGKEYS(                        XK_3,                      2                    )
    TAGKEYS(                        XK_4,                      3                    )
    TAGKEYS(                        XK_5,                      4                    )
    TAGKEYS(                        XK_6,                      5                    )
    TAGKEYS(                        XK_7,                      6                    )
    TAGKEYS(                        XK_8,                      7                    )
    TAGKEYS(                        XK_9,                      8                    )
    /*--- Kill ---------------------------------------------------------------------*/
    /* modifier                     key        function        argument             */
    { MODKEY|ShiftMask,             XK_x,      killunsel,      { 0 }                },
    { MODKEY|ShiftMask,             XK_c,      killclient,     { 0 }                },
    { Mod1Mask|Mod4Mask,            XK_l,      spawn,          { .v = lockcmd }     }, // lock
    { MODKEY|ShiftMask,             XK_r,      quit,           { 1 }                }, // restart
    { MODKEY|ControlMask|ShiftMask, XK_q,      quit,           { 0 }                }, // quit
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
    /* click                event mask      button          function        argument              */
    { ClkLtSymbol,          0,              Button1,        setlayout,      { 0 }                 },
    { ClkLtSymbol,          0,              Button3,        setlayout,      { .v = &layouts[2]  } },
    { ClkWinTitle,          0,              Button2,        zoom,           { 0 }                 },
    { ClkStatusText,        0,              Button1,        sigstatusbar,   { .i = 1 }            },
    { ClkStatusText,        0,              Button2,        sigstatusbar,   { .i = 2 }            },
    { ClkStatusText,        0,              Button3,        sigstatusbar,   { .i = 3 }            },
    { ClkClientWin,         MODKEY,         Button1,        movemouse,      { 0 }                 },
    { ClkClientWin,         MODKEY,         Button2,        togglefloating, { 0 }                 },
    { ClkClientWin,         MODKEY,         Button3,        resizemouse,    { 0 }                 },
    { ClkTagBar,            0,              Button1,        view,           { 0 }                 },
    { ClkTagBar,            0,              Button3,        toggleview,     { 0 }                 },
    { ClkTagBar,            MODKEY,         Button1,        tag,            { 0 }                 },
    { ClkTagBar,            MODKEY,         Button3,        toggletag,      { 0 }                 },
};

