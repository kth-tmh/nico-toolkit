/* ------------------------------------------------------------------------- */
/*                       Displays of RTDNN parameters                        */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/*   Copyright (C) 1994, 1995, 1996, 1997, 1998 Nikko Ström                  */
/*                                                                           */
/*   All rights reserved.                                                    */
/*                                                                           */
/*   Developed by Nikko Ström at the Dept. of Speech, Music and Hearing,     */
/*   KTH (Royal Institute of Technology)                                     */
/*                                                                           */
/*   Nikko Ström, nikko@speech.kth.se                                        */
/*                                                                           */
/*   KTH                                                                     */
/*   Institutionen för Tal, musik och hörsel                                 */
/*   S-100 44 STOCKHOLM                                                      */
/*   SWEDEN                                                                  */
/*                                                                           */
/*   http://www.speech.kth.se/                                               */
/*                                                                           */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/*         This software is part of the NICO toolkit for developing          */
/*                  Recurrent Time Delay Neural Networks                     */
/*                                                                           */
/* Permission to use, copy, or modify these programs and their documentation */
/* for educational and research purposes only and without fee is hereby      */
/* granted, provided that this copyright and permission notice appear on all */
/* copies and supporting documentation.  For any other uses of this software,*/
/* in original or modified form, including but not limited to distribution   */
/* in whole or in part, specific prior permission from the copyright holder  */
/* must be obtained. The copyright holder makes no representations about the */
/* suitability of this software for any purpose. It is provided "as is"      */
/* without express or implied warranty.                                      */
/*                                                                           */
/* ------------------------------------------------------------------------- */



void ShowNet        (Net, int info_level);

void ShowConnections(Net, int info_level);
void ShowCStat      (Net net, int all_flag, int level);

void ShowUnits      (Net, int info_level);
void ShowUStat      (Net net, int all_flag, int level);

void ShowObject     (Net, int obj, int info_level);

void ShowGroup      (Net, int group,        int info_level);
void ShowUnit       (Net, int unit,         int info_level);
void ShowStream     (Net, int stream,       int info_level);
void ShowConnection (Net, int from, int to, int info_level);

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  Display.h                                        */
/* --------------------------------------------------------------------------*/
