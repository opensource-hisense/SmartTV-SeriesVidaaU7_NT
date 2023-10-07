/*----------------------------------------------------------------------------*
 *MediaTek Inc. (C) 2019. All rights reserved.                                *
 *Copyright Statement:                                                        *
 *This software/firmware and related documentation ("MediaTek Software") are  *
 *protected under relevant copyright laws. The information contained herein is*
 *confidential and proprietary to MediaTek Inc. and/or its licensors. Without *
 *the prior written permission of MediaTek inc. and/or its licensors, any     *
 *reproduction, modification, use or disclosure of MediaTek Software, and     *
 *information contained herein, in whole or in part, shall be strictly        *
 *prohibited.                                                                 *
 *BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES *
 *THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")     *
 *RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER  *
 *ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL          *
 *WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED    *
 *WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR          *
 *NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH *
 *RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,            *
 *INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES*
 *TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.   *
 *RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO*
 *OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK       *
 *SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE  *
 *RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR     *
 *STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S *
 *ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE       *
 *RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE  *
 *MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE  *
 *CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.    *
 *The following software/firmware and/or related documentation ("MediaTek     *
 *Software") have been modified by MediaTek Inc. All revisions are subject to *
 *any receiver's applicable license agreements with MediaTek Inc.             *
 *---------------------------------------------------------------------------*/

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */


#include "nsEscCharsetProber.h"
#include "nsUniversalDetector.h"

nsEscCharSetProber::nsEscCharSetProber(PRUint32 aLanguageFilter)
{
  for (PRUint32 i = 0; i < NUM_OF_ESC_CHARSETS; i++)
    mCodingSM[i] = nsnull;
  if (aLanguageFilter & NS_FILTER_CHINESE_SIMPLIFIED) 
  {
    mCodingSM[0] = new nsCodingStateMachine(&HZSMModel);
    mCodingSM[1] = new nsCodingStateMachine(&ISO2022CNSMModel);
  }
  if (aLanguageFilter & NS_FILTER_JAPANESE)
    mCodingSM[2] = new nsCodingStateMachine(&ISO2022JPSMModel);
  if (aLanguageFilter & NS_FILTER_KOREAN)
    mCodingSM[3] = new nsCodingStateMachine(&ISO2022KRSMModel);
  mActiveSM = NUM_OF_ESC_CHARSETS;
  mState = eDetecting;
  mDetectedCharset = nsnull;
}

nsEscCharSetProber::~nsEscCharSetProber(void)
{
  for (PRUint32 i = 0; i < NUM_OF_ESC_CHARSETS; i++)
    delete mCodingSM[i];
}

void nsEscCharSetProber::Reset(void)
{
  mState = eDetecting;
  for (PRUint32 i = 0; i < NUM_OF_ESC_CHARSETS; i++)
    if (mCodingSM[i])
      mCodingSM[i]->Reset();
  mActiveSM = NUM_OF_ESC_CHARSETS;
  mDetectedCharset = nsnull;
}

nsProbingState nsEscCharSetProber::HandleData(const char* aBuf, PRUint32 aLen)
{
  PRUint32 codingState;
  PRInt32 j;
  PRUint32 i;

  for ( i = 0; i < aLen && mState == eDetecting; i++)
  {
    for (j = mActiveSM-1; j>= 0; j--)
    {
      if (mCodingSM[j])
      {
        codingState = mCodingSM[j]->NextState(aBuf[i]);
        if (codingState == eItsMe)
        {
          mState = eFoundIt;
          mDetectedCharset = mCodingSM[j]->GetCodingStateMachine();
          return mState;
        }
      }
    }
  }

  return mState;
}

