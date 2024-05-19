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
 * Module: edubtm_Split.c
 *
 * Description : 
 *  This file has three functions about 'split'.
 *  'edubtm_SplitInternal(...) and edubtm_SplitLeaf(...) insert the given item
 *  after spliting, and return 'ritem' which should be inserted into the
 *  parent page.
 *
 * Exports:
 *  Four edubtm_SplitInternal(ObjectID*, BtreeInternal*, Two, InternalItem*, InternalItem*)
 *  Four edubtm_SplitLeaf(ObjectID*, PageID*, BtreeLeaf*, Two, LeafItem*, InternalItem*)
 */


#include <string.h>
#include "EduBtM_common.h"
#include "BfM.h"
#include "EduBtM_Internal.h"



/*@================================
 * edubtm_SplitInternal()
 *================================*/
/*
 * Function: Four edubtm_SplitInternal(ObjectID*, BtreeInternal*,Two, InternalItem*, InternalItem*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  At first, the function edubtm_SplitInternal(...) allocates a new internal page
 *  and initialize it.  Secondly, all items in the given page and the given
 *  'item' are divided by halves and stored to the two pages.  By spliting,
 *  the new internal item should be inserted into their parent and the item will
 *  be returned by 'ritem'.
 *
 *  A temporary page is used because it is difficult to use the given page
 *  directly and the temporary page will be copied to the given page later.
 *
 * Returns:
 *  error code
 *    some errors caused by function calls
 *
 * Note:
 *  The caller should call BfM_SetDirty() for 'fpage'.
 */
Four edubtm_SplitInternal(
    ObjectID                    *catObjForFile,         /* IN catalog object of B+ tree file */
    BtreeInternal               *fpage,                 /* INOUT the page which will be splitted */
    Two                         high,                   /* IN slot No. for the given 'item' */
    InternalItem                *item,                  /* IN the item which will be inserted */
    InternalItem                *ritem)                 /* OUT the item which will be returned by spliting */
{
    Four                        e;                      /* error number */
    Two                         i;                      /* slot No. in the given page, fpage */
    Two                         j;                      /* slot No. in the splitted pages */
    Two                         k;                      /* slot No. in the new page */
    Two                         maxLoop;                /* # of max loops; # of slots in fpage + 1 */
    Four                        sum;                    /* the size of a filled area */
    Boolean                     flag=FALSE;             /* TRUE if 'item' become a member of fpage */
    PageID                      newPid;                 /* for a New Allocated Page */
    BtreeInternal               *npage;                 /* a page pointer for the new allocated page */
    Two                         fEntryOffset;           /* starting offset of an entry in fpage */
    Two                         nEntryOffset;           /* starting offset of an entry in npage */
    Two                         entryLen;               /* length of an entry */
    btm_InternalEntry           *fEntry;                /* internal entry in the given page, fpage */
    btm_InternalEntry           *nEntry;                /* internal entry in the new page, npage*/
    Boolean                     isTmp;

    // Overflow가 발생한 internal page를 split해 주어진 index entry를 삽입함.
    // split로 생간 index entry를 반환한다.

    // 1) 새로운 page를 할당 받고, internal page로 초기화한다.
    btm_AllocPage(catObjForFile, &fpage->hdr.pid, &newPid);
    edubtm_InitInternal(&newPid, FALSE, FALSE);
    // 2) 기존 index entry와 삽입할 index entry를 key 순으로 정렬한 뒤, 두 페이지에 나누어 저장한다.
    // a) 먼저, overflow가 발생한 page (fpage)에 index entry를 50% 정도 채운다.
    sum = 0;
    maxLoop = fpage->hdr.nSlots + 1;   
    j = 0;

    for (i = 0; i < maxLoop && sum < BI_HALF; i++) {
        // 삽입을 진행해야할 object가 fpage에 섞여들어가게 될 경우
        // item이 fpage의 멤버가 되었으므로 flag를 켜주어야한다. 
        if (i == high + 1) {
            entryLen = sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(item->klen);
            flag = TRUE;
        }
        else {
            fEntryOffset = fpage->slot[-j];
            fEntry = &fpage->data[fEntryOffset];
            
            entryLen = sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(item->klen);
            j++;
        }
        sum += entryLen + sizeof(Two);
    }
    fpage->hdr.nSlots = j;

    // b) 다음으로, split되어 새로 생긴 page에 남은 entry를 저장
    // 처음 저장되는 index entry는 할당 받은 page를 가리키는 internal index entry로 설정해 반환하고, p0 변수에 적어놓는다.
    for (k = -1; i < maxLoop; i++) {
        // 처음 저장되는 index entry에 대한 설정
        if (k == -1) {
            nEntryOffset = npage->hdr.free;
            nEntry = &npage->data[nEntryOffset];

            if (!flag && i == high + 1) {
                entryLen = sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(item->klen);
                memcpy(ritem, item, entryLen);
            }
            else {
                fEntryOffset = fpage->slot[-j];
                fEntry = &fpage->data[fEntryOffset];

                entryLen = sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(fEntry->klen);
                memcpy(ritem, fEntry, entryLen);

                if (fEntryOffset + entryLen == fpage->hdr.free) fpage->hdr.free -= entryLen;
                else fpage->hdr.unused += entryLen;

                j++;
            }
            npage->hdr.p0 = ritem->spid;
        }
        else {
            nEntryOffset = npage->hdr.free;
            npage->slot[-k] = nEntryOffset;
            nEntry = &npage->data[nEntryOffset];

            if (!flag && i == high + 1) {
                entryLen = sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(item->klen);
                memcpy(nEntry, item, entryLen);
            }
            else {
                fEntryOffset = fpage->slot[-j];
                fEntry = &fpage->data[fEntryOffset];

                entryLen = sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(fEntry->klen);
                memcpy(nEntry, fEntry, entryLen);

                if (fEntryOffset + entryLen == fpage->hdr.free) fpage->hdr.free -= entryLen;
                else fpage->hdr.unused += entryLen;

                j++;
            }
            npage->hdr.free += entryLen;
        }
        k++;
    }
    npage->hdr.nSlots = k;

    // fpage 업데이트
    if (flag == TRUE) {
        for (i = fpage->hdr.nSlots - 1; i > high; i--) fpage->slot[-(i+1)] = fpage->slot[-i];
        fpage->slot[-(high + 1)] = fpage->hdr.free;

        fEntry = &fpage->data[fpage->hdr.free];
        entryLen = sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(item->klen);

        fpage->hdr.free += entryLen;
        fpage->hdr.nSlots++;
    }

    // split된 page가 root일 경우, type을 internal로 변경해준다.
    if (fpage->hdr.type & ROOT) fpage->hdr.type = INTERNAL;

    // 마무리
    BfM_SetDirty(&newPid, PAGE_BUF);
    BfM_FreeTrain(&newPid, PAGE_BUF);
    
    return(eNOERROR);
    
} /* edubtm_SplitInternal() */



/*@================================
 * edubtm_SplitLeaf()
 *================================*/
/*
 * Function: Four edubtm_SplitLeaf(ObjectID*, PageID*, BtreeLeaf*, Two, LeafItem*, InternalItem*)
 *
 * Description: 
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  The function edubtm_SplitLeaf(...) is similar to edubtm_SplitInternal(...) except
 *  that the entry of a leaf differs from the entry of an internal and the first
 *  key value of a new page is used to make an internal item of their parent.
 *  Internal pages do not maintain the linked list, but leaves do it, so links
 *  are properly updated.
 *
 * Returns:
 *  Error code
 *  eDUPLICATEDOBJECTID_BTM
 *    some errors caused by function calls
 *
 * Note:
 *  The caller should call BfM_SetDirty() for 'fpage'.
 */
Four edubtm_SplitLeaf(
    ObjectID                    *catObjForFile, /* IN catalog object of B+ tree file */
    PageID                      *root,          /* IN PageID for the given page, 'fpage' */
    BtreeLeaf                   *fpage,         /* INOUT the page which will be splitted */
    Two                         high,           /* IN slotNo for the given 'item' */
    LeafItem                    *item,          /* IN the item which will be inserted */
    InternalItem                *ritem)         /* OUT the item which will be returned by spliting */
{
    Four                        e;              /* error number */
    Two                         i;              /* slot No. in the given page, fpage */
    Two                         j;              /* slot No. in the splitted pages */
    Two                         k;              /* slot No. in the new page */
    Two                         maxLoop;        /* # of max loops; # of slots in fpage + 1 */
    Four                        sum;            /* the size of a filled area */
    PageID                      newPid;         /* for a New Allocated Page */
    PageID                      nextPid;        /* for maintaining doubly linked list */
    BtreeLeaf                   tpage;          /* a temporary page for the given page */
    BtreeLeaf                   *npage;         /* a page pointer for the new page */
    BtreeLeaf                   *mpage;         /* for doubly linked list */
    btm_LeafEntry               *itemEntry;     /* entry for the given 'item' */
    btm_LeafEntry               *fEntry;        /* an entry in the given page, 'fpage' */
    btm_LeafEntry               *nEntry;        /* an entry in the new page, 'npage' */
    ObjectID                    *iOidArray;     /* ObjectID array of 'itemEntry' */
    ObjectID                    *fOidArray;     /* ObjectID array of 'fEntry' */
    Two                         fEntryOffset;   /* starting offset of 'fEntry' */
    Two                         nEntryOffset;   /* starting offset of 'nEntry' */
    Two                         oidArrayNo;     /* element No in an ObjectID array */
    Two                         alignedKlen;    /* aligned length of the key length */
    Two                         itemEntryLen;   /* length of entry for item */
    Two                         entryLen;       /* entry length */
    Boolean                     flag;
    Boolean                     isTmp;
 
    // Overflow가 발생한 leaf page를 split하여 index entry를 삽입 후
    // 새 leaf page를 가리키는 index entry를 반환한다.

    // 1) 새로운 page를 할당 받고, internal page로 초기화한다.
    btm_AllocPage(catObjForFile, &fpage->hdr.pid, &newPid);
    edubtm_InitLeaf(&newPid, FALSE, FALSE);
    // 2) 기존 index entry들과 삽입할 index entry를 key 순으로 정렬한 뒤, 나누어 저장한다.
    // a) 먼저, overflow가 발생한 page (fpage)에 index entry를 50% 정도 채운다.
    sum = 0;
    maxLoop = fpage->hdr.nSlots + 1;   
    j = 0;

    for (i = 0; i < maxLoop && sum < BI_HALF; i++) {
        // 삽입을 진행해야할 object가 fpage에 섞여들어가게 될 경우
        // item이 fpage의 멤버가 되었으므로 flag를 켜주어야한다. 
        if (i == high + 1) {
            entryLen = sizeof(ShortPageID) + 2 * sizeof(Two) + ALIGNED_LENGTH(item->klen);
            flag = TRUE;
        }
        else {
            fEntryOffset = fpage->slot[-j];
            fEntry = &fpage->data[fEntryOffset];
            
            entryLen = sizeof(ShortPageID) + 2 * sizeof(Two) + ALIGNED_LENGTH(item->klen);
            j++;
        }
        sum += entryLen + sizeof(Two);
    }
    fpage->hdr.nSlots = j;

    // b) 다음으로, split되어 새로 생긴 page에 남은 entry를 저장
    for (k = 0; i < maxLoop; i++) {
        nEntryOffset = npage->hdr.free;
        npage->slot[-k] = nEntryOffset;
        nEntry = &npage->data[nEntryOffset];

        if (!flag && i == high + 1) {
            entryLen = sizeof(ShortPageID) + 2 * sizeof(Two) + ALIGNED_LENGTH(item->klen);

            nEntry->nObjects = item->nObjects;
            nEntry->klen = item->klen;

            memcpy(nEntry->kval, item->kval, item->klen);
            memcpy(&nEntry->kval[ALIGNED_LENGTH(item->klen)], &item->oid, OBJECTID_SIZE);
        }
        else {
            fEntryOffset = fpage->slot[-j];
            fEntry = &fpage->data[fEntryOffset];

            entryLen = sizeof(ShortPageID) + 2 * sizeof(Two) + ALIGNED_LENGTH(fEntry->klen);
            memcpy(nEntry, fEntry, entryLen);

            if (fEntryOffset + entryLen == fpage->hdr.free) fpage->hdr.free -= entryLen;
            else fpage->hdr.unused += entryLen;

            j++;
        }
        npage->hdr.free += entryLen;
        k++;
    }
    npage->hdr.nSlots = k;

    // fpage 업데이트
    if (flag == TRUE) {
        for (i = fpage->hdr.nSlots - 1; i > high; i--) fpage->slot[-(i+1)] = fpage->slot[-i];
        fpage->slot[-(high + 1)] = fpage->hdr.free;

        fEntry = &fpage->data[fpage->hdr.free];
        fEntry->nObjects = item->nObjects;
        fEntry->klen = item->klen;

        memcpy(fEntry->kval, item->kval, item->klen);
        memcpy(&fEntry->kval[ALIGNED_LENGTH(item->klen)], &item->oid, OBJECTID_SIZE);

        entryLen = sizeof(ShortPageID) + 2 * sizeof(Two) + ALIGNED_LENGTH(item->klen);

        fpage->hdr.free += entryLen;
        fpage->hdr.nSlots++;
    }

    // 3) 할당 받은 page를 leaf page들로 구성된 doubly linked list에 추가한다.
    npage->hdr.prevPage = root->pageNo;
    npage->hdr.nextPage = fpage->hdr.nextPage;
    fpage->hdr.nextPage = newPid.pageNo;

    if (npage->hdr.nextPage != NIL) {
        nextPid.pageNo = npage->hdr.nextPage;
        nextPid.volNo = npage->hdr.pid.volNo;

        BfM_GetTrain(&nextPid, &mpage, PAGE_BUF);
        mpage->hdr.prevPage = newPid.pageNo;
        
        BfM_SetDirty(&nextPid, PAGE_BUF);
        BfM_FreeTrain(&nextPid, PAGE_BUF);
    }

    // 5) 할당 받은 page를 가리키는 index entry 생성
    nEntry = &npage->data[npage->slot[0]];
    ritem->spid = newPid.pageNo;
    ritem->klen = nEntry->klen;
    memcpy(ritem->kval, nEntry->kval, nEntry->klen);

    // split된 page가 root일 경우, type을 LEAF로 변경해준다.
    if (fpage->hdr.type & ROOT) fpage->hdr.type = LEAF;

    // 마무리
    BfM_SetDirty(&newPid, PAGE_BUF);
    BfM_FreeTrain(&newPid, PAGE_BUF);

    return(eNOERROR);
    
} /* edubtm_SplitLeaf() */
