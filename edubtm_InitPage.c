/******************************************************************************/
/*                                                                            */
/*    ODYSSEUS/EduCOSMOS Educational-Purpose Object Storage System            */
/*                                                                            */
/*    Developed by Professor Kyu-Young Whang et al.                           */
/*                                                                            */
/*    Database and Multimedia Laboratory                                      */
/*                                                                            */
/*    Computer Science Department and                                         */
/*    Advanced Information Technology Research Center (AITrc)                 */
/*    Korea Advanced Institute of Science and Technology (KAIST)              */
/*                                                                            */
/*    e-mail: kywhang@cs.kaist.ac.kr                                          */
/*    phone: +82-42-350-7722                                                  */
/*    fax: +82-42-350-8380                                                    */
/*                                                                            */
/*    Copyright (c) 1995-2013 by Kyu-Young Whang                              */
/*                                                                            */
/*    All rights reserved. No part of this software may be reproduced,        */
/*    stored in a retrieval system, or transmitted, in any form or by any     */
/*    means, electronic, mechanical, photocopying, recording, or otherwise,   */
/*    without prior written permission of the copyright owner.                */
/*                                                                            */
/******************************************************************************/
/*
 * Module: edubtm_InitPage.c
 *
 * Description :
 *  This file has three functions which initialize Btree pages.  A Btree page
 *  should be initialized by one of these functions before it is used.
 *
 * Exports:
 *  Four edubtm_InitInternal(PageID*, Boolean)
 *  Four edubtm_InitLeaf(PageID*, Boolean)
 */


#include "EduBtM_common.h"
#include "BfM.h"
#include "EduBtM_Internal.h"



/*@================================
 * edubtm_InitInternal()
 *================================*/
/*
 * Function: Four edubtm_InitInternal(PageID*, Boolean)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  Initialize as an internal page.  If 'root' is TRUE, this page may be
 *  initialized as a root.
 *
 * Returns:
 *  Error code
 *    some errors caused by function calls
 */
Four edubtm_InitInternal(
    PageID  *internal,		/* IN the PageID to be initialized */
    Boolean root,		/* IN Is it root ? */
    Boolean isTmp)              /* IN Is it temporary ? - COOKIE12FEB98 */
{
    Four e;			/* error number */
    BtreeInternal *page;	/* a page pointer */

    // page를 B+ tree의 internal page로 초기화한다.
    // 1. page header를 internal page로 초기화한다.
    
    BfM_GetNewTrain(internal, (char**)&page, PAGE_BUF);

    // pid: 파라미터로 주어진 값을 이용
    page->hdr.pid = *internal;
    // flags: B+ tree index page임을 나타내는 bit을 세팅
    page->hdr.flags = BTREE_PAGE_TYPE;
    // type: internal page를 나타내는 bit을 세팅
    //       root page라면 root page임을 나타내는 bit도 세팅
    if (root) page->hdr.type = INTERNAL | ROOT;
    else page->hdr.type = INTERNAL;

    page->hdr.p0 = NIL;
    page->hdr.nSlots = 0;
    page->hdr.free = 0;
    page->hdr.unused = 0;

    // 2. 마무리
    BfM_SetDirty(internal, PAGE_BUF);
    BfM_FreeTrain(internal, PAGE_BUF);
    
    return(eNOERROR);
    
}  /* edubtm_InitInternal() */



/*@================================
 * edubtm_InitLeaf()
 *================================*/
/*
 * Function: Four edubtm_InitLeaf(PageID*, Boolean)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  Initialize as a leaf page. If 'root' is TRUE, this page may be initialize
 *  as a root.
 *
 * Returns:
 *  Error code
 *    some errors caused by function calls
 */
Four edubtm_InitLeaf(
    PageID *leaf,		/* IN the PageID to be initialized */
    Boolean root,		/* IN Is it root ? */
    Boolean isTmp)              /* IN Is it temporary ? */
{
    Four e;			/* error number */
    BtreeLeaf *page;		/* a page pointer */

    // page를 B+ tree 의 leaf page로 초기화한다.
    BfM_GetNewTrain(leaf, (char**)&page, PAGE_BUF);

    page->hdr.pid = *leaf;
    
    //flags: Set a bit indicating that the page is a B+ tree index page
    page->hdr.flags = BTREE_PAGE_TYPE;

    //type: Set a bit indicating that the page is a leaf page
    if (root) page->hdr.type = LEAF | ROOT;
    else page->hdr.type = LEAF;

    page->hdr.nSlots = 0;
    page->hdr.free = 0;
    page->hdr.prevPage = NIL;
    page->hdr.nextPage = NIL;
    page->hdr.unused = 0;

    BfM_SetDirty(leaf, PAGE_BUF);
    BfM_FreeTrain(leaf, PAGE_BUF);
    
    return(eNOERROR);
    
}  /* edubtm_InitLeaf() */
