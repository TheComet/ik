%%
%% This is file `fhnwreport.cls',
%% generated with the docstrip utility.
%%
%% The original source files were:
%%
%% fhnwreport.dtx 
%% 
%% IMPORTANT NOTICE:
%% 
%% For the copyright see the source file.
%% 
%% Any modified versions of this file must be renamed
%% with new filenames distinct from fhnwreport.cls.
%% 
%% For distribution of the original source see the terms
%% for copying and modification in the file fhnwreport.dtx.
%% 
%% This generated file may be distributed as long as the
%% original source files, as listed above, are part of the
%% same distribution. (The sources need not necessarily be
%% in the same archive or directory.)
\NeedsTeXFormat{LaTeX2e}[1995/12/01]
\ProvidesClass{fhnwreport}[2013/10/07 v2.0 PDF-LaTeX class for FHNW reports]
\LoadClass{article}
\RequirePackage{geometry}
\RequirePackage{calc}
\RequirePackage{times}
\RequirePackage{graphicx}
\geometry{a4paper,twoside=true}
\geometry{width=160mm,height=241mm,top=32mm,outer=25mm}
\geometry{footskip=19mm,headsep=13mm}
\newcommand{\CaptionFont}{\em}
\newcommand{\SecFont}{\sf\bfseries}
\newcommand{\SecNumFont}{\sf\bfseries}
\newcommand{\ChapFont}{\sf\bfseries}
\newcommand{\TitleFont}{\sf\bfseries}
\newcommand{\HeaderFont}{\sf\small}
\newcommand{\BackgroundFont}{\em}
\newcommand{\DTfont@author}{\sf\bfseries}
\newcommand{\DTfont@normal}{\sf}
\newcommand{\DTfont@series}{\sf\bfseries}
\newcommand{\DTfont@title}{\sf\bfseries}
\renewcommand\maketitle{\par
  \begingroup
    \renewcommand\thefootnote{\@fnsymbol\c@footnote}%
    \def\@makefnmark{\rlap{\@textsuperscript{\normalfont\@thefnmark}}}%
    \long\def\@makefntext##1{\parindent 1em\noindent
            \hb@xt@1.8em{%
                \hss\@textsuperscript{\normalfont\@thefnmark}}##1}%
    \if@twocolumn
      \ifnum \col@number=\@ne
        \@maketitle
      \else
        \twocolumn[\@maketitle]%
      \fi
    \else
      \newpage
      \global\@topnum\z@   % Prevents figures from going at top of the title page.
      \@maketitle
    \fi
    \thispagestyle{empty}\@thanks
  \endgroup
  \setcounter{footnote}{0}%
  \global\let\thanks\relax
  \global\let\maketitle\relax
  \global\let\@maketitle\relax
  \global\let\@thanks\@empty
  \global\let\@author\@empty
  \global\let\@date\@empty
  \global\let\@title\@empty
  \global\let\title\relax
  \global\let\author\relax
  \global\let\date\relax
  \global\let\and\relax
}
\def\@maketitle{%
  \newpage
  \null
  \setlength{\unitlength}{1cm}%
%  \begin{picture}(0,0)
%    \linethickness{0.025mm}
%    \put(-0.85,2.2){\mbox{\includegraphics[height=12mm]{fhnwlogo.pdf}}}
%  \end{picture}
  \par%
  \vskip 2em%
  \begin{center}%
    \let \footnote \thanks
    {\LARGE \@title \par}%
    \vskip 1.5em%
    {\large
      \lineskip .5em%
      \begin{tabular}[t]{c}%
        \@author
      \end{tabular}\par}%
    \vskip 1em%
    {\large \@date}%
  \end{center}%
  \par
  \vskip 1.5em}
\newcommand{\noparskip}{\parskip=\z@ \@plus\z@ \@minus\z@}
\ifx\TextAlignment\undefined
  \newcommand{\TextAlignment}{\relax}
  \parindent=0pt
\else\fi
\AtBeginDocument{\TextAlignment}
\parskip=1.6ex \@plus 0.4ex \@minus 0.4ex
\widowpenalty 10000
\clubpenalty 10000
\@clubpenalty 10000
\def\@listi{\leftmargin\leftmargini
  \parsep \z@ \@plus2\p@ \@minus\z@
  \topsep \z@ \@plus2\p@ \@minus\z@
  \itemsep\z@ \@plus2\p@ \@minus\z@}
\let\@listI\@listi
\@listi
\def\@listii{\leftmargin\leftmarginii
  \labelwidth\leftmarginii
  \advance\labelwidth-\labelsep
  \topsep \z@ \@plus2\p@  \@minus\z@
  \parsep \z@ \@plus\p@ \@minus\z@
  \itemsep \parsep}
\def\@listiii{\leftmargin\leftmarginiii
  \labelwidth\leftmarginiii
  \advance\labelwidth-\labelsep
  \topsep \z@ \@plus\p@ \@minus\z@
  \parsep \z@ \@minus\z@
  \partopsep \z@ \plus\z@ \@minus\z@
  \itemsep \topsep}
\def\@listiv {\leftmargin\leftmarginiv
  \labelwidth\leftmarginiv
  \advance\labelwidth-\labelsep}
\def\@listv  {\leftmargin\leftmarginv
  \labelwidth\leftmarginv
  \advance\labelwidth-\labelsep}
\def\@listvi {\leftmargin\leftmarginvi
  \labelwidth\leftmarginvi
  \advance\labelwidth-\labelsep}
\renewcommand\normalsize{%
   \@setfontsize\normalsize\@xipt{13.6}%
   \abovedisplayskip 11\p@ \@plus3\p@ \@minus6\p@
   \abovedisplayshortskip \z@ \@plus3\p@
   \belowdisplayshortskip 6.5\p@ \@plus3.5\p@ \@minus3\p@
   \belowdisplayskip \abovedisplayskip
   \let\@listi\@listI}
\normalsize
\renewcommand\small{%
   \@setfontsize\small\@xpt\@xiipt
   \abovedisplayskip 10\p@ \@plus2\p@ \@minus5\p@
   \abovedisplayshortskip \z@ \@plus3\p@
   \belowdisplayshortskip 6\p@ \@plus3\p@ \@minus3\p@
   \def\@listi{\leftmargin\leftmargini
               \topsep \z@ \@plus2\p@ \@minus\z@
               \parsep \z@ \@plus\p@ \@minus\z@
               \itemsep \parsep}%
   \belowdisplayskip \abovedisplayskip
}
\renewcommand\footnotesize{%
   \@setfontsize\footnotesize\@ixpt{11}%
   \abovedisplayskip 8\p@ \@plus2\p@ \@minus4\p@
   \abovedisplayshortskip \z@ \@plus\p@
   \belowdisplayshortskip 4\p@ \@plus2\p@ \@minus2\p@
   \def\@listi{\leftmargin\leftmargini
               \topsep \z@ \@plus\p@ \@minus\z@
               \parsep \z@ \@plus\p@ \@minus\z@
               \itemsep \parsep}%
   \belowdisplayskip \abovedisplayskip
}
\renewcommand\scriptsize{\@setfontsize\scriptsize\@viiipt{9.5}}
\renewcommand\tiny{\@setfontsize\tiny\@vipt\@viipt}
\renewcommand\large{\@setfontsize\large\@xiipt{14}}
\renewcommand\Large{\@setfontsize\Large\@xivpt{18}}
\renewcommand\LARGE{\@setfontsize\LARGE\@xviipt{22}}
\renewcommand\huge{\@setfontsize\huge\@xxpt{25}}
\renewcommand\Huge{\@setfontsize\Huge\@xxvpt{30}}
\def\ps@headings{%
  \let\@oddfoot\@empty\let\@evenfoot\@empty
  \def\@evenhead{\thepage\hfil\slshape\leftmark}%
  \def\@oddhead{{\slshape\rightmark}\hfil\thepage}%
  \let\@mkboth\markboth
  \def\sectionmark##1{%
    \markboth {\MakeUppercase{%
        \ifnum \c@secnumdepth >\z@
        \thesection\quad
        \fi
        ##1}}{}}%
  \def\subsectionmark##1{%
    \markright {%
      \ifnum \c@secnumdepth >\@ne
      \thesubsection\quad
      \fi
      ##1}}}
\pagestyle{headings}
\renewcommand\section{\@startsection {section}{1}{\z@}%
  {-2.1ex \@plus -.6ex \@minus -.12ex}%
  {1.38ex \@plus.12ex}%
  {\SecFont\Large}}
\renewcommand\subsection{\@startsection{subsection}{2}{\z@}%
  {-1.95ex\@plus -.6ex \@minus -.12ex}%
  {.9ex \@plus .12ex}%
  {\SecFont\large}}
\renewcommand\subsubsection{\@startsection{subsubsection}{3}{\z@}%
  {-1.95ex\@plus -.6ex \@minus -.12ex}%
  {.9ex \@plus .12ex}%
  {\SecFont\normalsize}}
\renewcommand{\paragraph}[1]{\noindent\textbf{#1}}
\def\@seccntformat#1{\csname the#1\endcsname\quad}
\RequirePackage{hyperref,color}
\hypersetup{bookmarksnumbered={true},colorlinks={true},linkcolor=black,citecolor=black}
\RequirePackage{caption}
\captionsetup{margin=10pt,font=small,labelfont=bf}
\setlength\floatsep    {24\p@ \@plus 2\p@ \@minus 4\p@}
\setlength\textfloatsep{30\p@ \@plus 2\p@ \@minus 4\p@}
\endinput
%%
%% End of file `fhnwreport.cls'.
