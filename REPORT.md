# EduBtM Report

Name: 오형민

Student id: 20242502

# Problem Analysis

EduCOSMOS에서 B+ tree index manager를 구현하는 것을 목표로 한다.
구체적으로 B+ tree index의 생성/소멸, 삽입/삭제/스캔 연산을 구현한다.

# Design For Problem Solving

## High Level

1. EduBtM_CreateIndex
index file에서 B+ tree index를 생성한다.
+ btm_AllocPage를 호출해 index file의 첫 page를 할당받은 뒤 root page로 초기화한다.

2. EduBtM_DropIndex
index file에서 B+ tree index를 삭제한다.
+ B+ tree index의 모든 page를 deallocate한다.

3. EduBtM_InsertIndex
B+ tree index에 새로운 object를 삽입한다.
+ edubtm_Insert를 호출해 (key, ID) pair를 B+ tree index에 삽입한다.
+ root page에서 split이 발생한 경우, edubtm_root_insert를 호출해 처리한다.

4. EduBtM_DeleteIndex
B+ tree index에서 object를 삭제한다.
+ edubtm_Delete를 호출해 (key, ID) pair를 B+ tree index에서 삭제한다.
+ underflow가 발생한 경우 btm_root_delete를 호출해 이를 처리한다.
+ root page에서 split이 발생한 경우, edubtm_root_insert를 호출해 처리한다.

5. EduBtM_Fetch
B+ tree index에서 검색 조건을 만족하는 첫 object를 검색한다.
+ startCompOp가 SM_BOF일 경우, 첫 object를 검색한다. 
+ startCompOp가 SM_EOF일 경우, 마지막 object를 검색한다.
+ 이외의 경우, edubtm_Fetch를 호출해 검색을 처리한다.

6. EduBtM_FetchNext
B+ tree index에서 현재 object의 다음 object를 검색한다.
+ edubtm_FetchNext를 호출해 검색 조건을 만족하는 index entry의 다음 entry를 검색함.

## Low Level

1. edubtm_InitLeaf
page를 leaf page로 초기화한다.
+ page header를 초기화한다.

2. edubtm_InitInternal
page를 internal page로 초기화한다.
+ page header를 초기화한다.

3. edubtm_FreePages
index page의 subtree를 deallocation 시킨다.
+ 모든 자식 page들에 대해 재귀적으로 edubtm_FreePages()를 호출해 처리한다.

4. edubtm_Insert
파라미터로 주어진 page를 root page로 하는 B+ tree index에서 새 object에 대한 (key, ID) pair를 삽입 한다.
+ root page가 internal page일 경우
++ 새 object를 삽입할 leaf page를 찾기 위해 이분탐색을 수행하고
++ 결정된 자식 page를 root page로 하는 B+ subtree에 재귀적으로 edubtm_Insert를 호출한다.
++ 자식 page에서 split이 발생하면 split 함수를 호출해 처리한다.
+ root page가 leaf page일 경우
++ edubtm_InsertLeaf를 호출해 (key, ID) pair를 삽입함
++ split이 발생한 경우, edubtm_SplitLeaf 함수를 호출해 처리한다.

5. edubtm_InsertLeaf
Leaf page에 새 index entry를 삽입한다.
+ 삽입할 slotNo를 결정하고, 필요한 free space를 계산한다.
+ 필요 시 compaction을 진행하고, entry를 삽입한다.
+ page overflow가 발생하면 edubtm_SplitLeaf를 호출해 처리한다.

6. edubtm_InsertInternal
Internal page에 새 index entry를 삽입한다.
+ 필요한 free space를 계산한다.
+ 필요 시 compaction을 진행하고, entry를 삽입한다.
+ page overflow가 발생하면 edubtm_SplitLeaf를 호출해 처리한다.

7. edubtm_SplitLeaf
Overflow가 발생한 leaf page를 split해 파라미터로 주어진 index entry를 삽입한다.
+ 새로운 page를 할당 받은 뒤, leaf page로 초기화 함
+ 기존 index entry와 삽입할 index entry를 key 순으로 정렬해 균등하게 두 페이지에 나누어담는다.
+ 새로 할당 받은 page를 doubly linked list에 추가하고, index entry를 생성한다.

8. edubtm_SplitInternal
Overflow가 발생한 internal page를 split해 파라미터로 주어진 index entry를 삽입한다.
+ 새로운 page를 할당 받은 뒤, internal page로 초기화 함
+ 기존 index entry와 삽입할 index entry를 key 순으로 정렬해 균등하게 두 페이지에 나누어담는다.
+ 새로 할당 받은 page를 doubly linked list에 추가하고, index entry를 생성한다.

9. edubtm_root_insert
Root page가 split될 경우, 새로운 root page를 생성한다.
+ 새 page를 할당받고, 기존 root page를 복사한다.
+ 기존 root page는 새 root page로 초기화하고, 새 root page를 자식으로 넣어준다.

10. edubtm_Delete
파라미터로 주어진 page가 root page인 B+ tree index에서 (key, ID)를 삭제한다.
+ root page가 internal page일 경우
++ (key, ID)가 저장된 leaf page를 찾기 위해 들어가야할 자식 노드를 이분탐색으로 찾는다.
++ 재귀적으로 edubtm_Delete를 호출해 삭제를 수행한다.
++ underflow가 발생한 경우, btm_Underflow를 호출해 이를 처리한다.
+++ underflow가 발생한 자식 page의 부모 page가 overflow가 발생한 경우, edubtm_insertInternal을 호출해 처리한다.
+ root page가 leaf page일 경우
++ edubtm_DeleteLeaf를 호출해 (key, ID)를 삭제한다.
++ underflow가 발생할 경우, output parameter f를 TRUE로 설정한다.

11. edubtm_DeleteLeaf
Leaf page에서 (key, ID) pair를 삭제함.
+ (key, ID)가 저장된 index entry에서 slot을 삭제한다.
+ slot array에 빈공간이 없도록 compaction을 진행하고 header를 갱신한다.
+ underflow가 발생한 경우, output parameter f를 TRUE로 설정한다. 

12. edubtm_CompactLeafPage
Leaf page의 free space 사이에 빈 공간이 없도록 offset을 조정한다.
+ slotNo가 NIL이면 모든 index entry를 앞에서부터 저장한다.
+ slotNo가 NIL이 아니면 slotNo를 제외한 index entry를 앞에서 연속되게 저장하고 slotNo에 대응하는 index entry를 마지막 위치에 저장한다.

13. edubtm_CompactInternalPage
Internal page의 free space 사이에 빈 공간이 없도록 offset을 조정한다.
+ slotNo가 NIL이면 모든 index entry를 앞에서부터 저장한다.
+ slotNo가 NIL이 아니면 slotNo를 제외한 index entry를 앞에서 연속되게 저장하고 slotNo에 대응하는 index entry를 마지막 위치에 저장한다.

14. edubtm_Fetch
B+ tree index에서 검색 조건을 만족하는 첫 entry를 검색한다.
+ 주어진 root page가 internal page일 경우
++ 이분탐색을 수행해 다음으로 방문할 자식 page를 결정한다.
++ 해당 자식 page를 root page로 하는 B+ subtree에서 leaf entry를 검색하기 위해 재귀적으로 edubtm_Fetch를 호출한다.
+ 주어진 root page가 leaf page일 경우
++ 이분탐색을 활용해 (key, ID)가 저장된 index entry를 검색한다

15. edubtm_FetchNext
B+ tree index에서 검색 조건을 만족하는 entry의 다음 leaf entry를 검색 후 cursor를 반환함.

16. edubtm_FirstObject
B+ tree index에서 첫 object를 검색한다.
+ 검색 종료 key 값이 마지막 object보다 작거나, key 값은 같지만 종료 연산이 SM_LT일 때 CURSOR_EOS를 반환한다.

17. edubtm_LastObject
B+ tree index에서 마지막 object를 검색한다.
+ 검색 종료 key 값이 마지막 object보다 크거나, key 값은 같지만 종료 연산이 SM_GT일 때 CURSOR_EOS를 반환한다.

18. edubtm_BinarySearchLeaf
Leaf page에서 파라미터로 주어진 key보다 작거나 같은 key 값을 갖는 index entry를 검색 후 위치를 반환한다.
+ 주어진 key 값과 같은 key 값을 갖는 index entry가 존재하면 slotNo와 TRUE를 반환한다.
+ 주어진 key 값과 같은 key 값을 갖는 index entry가 존재하지 않으면 그 이하의 값을 갖는 entry 중 가장 큰 entry의 slotNo와 FALSE를 반환한다. 작은 key 값을 갖는 index entry가 없으면 -1을 반환한다. 

19. edubtm_BinarySearchInternal
Internal page에서 파라미터로 주어진 key보다 작거나 같은 key 값을 갖는 index entry를 검색 후 위치를 반환한다.
+ 주어진 key 값과 같은 key 값을 갖는 index entry가 존재하면 slotNo와 TRUE를 반환한다.
+ 주어진 key 값과 같은 key 값을 갖는 index entry가 존재하지 않으면 그 이하의 값을 갖는 entry 중 가장 큰 entry의 slotNo와 FALSE를 반환한다. 작은 key 값을 갖는 index entry가 없으면 -1을 반환한다. 

20. edubtm_KeyCompare
파라미터로 주어진 두 key 값을 비교해 EQUAL, GREAT, LESS를 반환한다.


# Mapping Between Implementation And the Design
## High Level
1. EduBtM_CreateIndex
```
Four EduBtM_CreateIndex(
    ObjectID *catObjForFile,	/* IN catalog object of B+ tree file */
    PageID *rootPid)		/* OUT root page of the newly created B+tree */
{
    Four e;			/* error number */
    Boolean isTmp;
    SlottedPage *catPage;	/* buffer page containing the catalog object */
    sm_CatOverlayForBtree *catEntry; /* pointer to Btree file catalog information */
    PhysicalFileID pFid;	/* physical file ID */

    // inde file에 새로운 index를 생성하고, 생성한 index의 root page ID를 반환
    // 관련 함수: btm_AllocPage, BfM_GetTrain, BfM_FreeTrain
    
    // 1. btm_AllocPage()를 호출해 index file의 첫 페이지를 allocation
    BfM_GetTrain((TrainID*)catObjForFile, (char**)&catPage, PAGE_BUF);
    
    GET_PTR_TO_CATENTRY_FOR_BTREE(catObjForFile, catPage, catEntry);
    MAKE_PAGEID(pFid, catEntry->fid.volNo, catEntry->firstPage);

    e = btm_AllocPage(catObjForFile, (PageID*)&pFid, rootPid);
    if (e < 0) ERR(e);

    // 2. allocated page를 root page로 초기화
    e = edubtm_InitLeaf(rootPid, TRUE, FALSE);
    if (e < 0) ERR(e);

    // 3. page ID를 반환
    // 할당된 object 정리
    e = BfM_FreeTrain((TrainID*)catObjForFile, PAGE_BUF);
    if (e < 0) ERR(e);
    
    return(eNOERROR);
    
} /* EduBtM_CreateIndex() */
```

2. EduBtM_DropIndex
```
Four EduBtM_DropIndex(
    PhysicalFileID *pFid,	/* IN FileID of the Btree file */
    PageID *rootPid,		/* IN root PageID to be dropped */
    Pool   *dlPool,		/* INOUT pool of the dealloc list elements */
    DeallocListElem *dlHead) /* INOUT head of the dealloc list */
{
    Four e;			/* for the error number */

    /*@ Free all pages concerned with the root. */
    // Delete a B+ tree index from an index file
    // Deallocate a root page and every child page of the B+ tree index
    // 연관 함수: edubtm_FreePages
    edubtm_FreePages(pFid, rootPid, dlPool, dlHead);
	
    return(eNOERROR);
    
} /* EduBtM_DropIndex() */
```

3. EduBtM_InsertIndex
```
Four EduBtM_InsertObject(
    ObjectID *catObjForFile,	/* IN catalog object of B+ tree file */
    PageID   *root,		/* IN the root of Btree */
    KeyDesc  *kdesc,		/* IN key descriptor */
    KeyValue *kval,		/* IN key value */
    ObjectID *oid,		/* IN ObjectID which will be inserted */
    Pool     *dlPool,		/* INOUT pool of dealloc list */
    DeallocListElem *dlHead) /* INOUT head of the dealloc list */
{
    int i;
    Four e;			/* error number */
    Boolean lh;			/* for spliting */
    Boolean lf;			/* for merging */
    InternalItem item;		/* Internal Item */
    SlottedPage *catPage;	/* buffer page containing the catalog object */
    sm_CatOverlayForBtree *catEntry; /* pointer to Btree file catalog information */
    PhysicalFileID pFid;	 /* B+-tree file's FileID */

    
    /*@ check parameters */
    
    if (catObjForFile == NULL) ERR(eBADPARAMETER_BTM);
    
    if (root == NULL) ERR(eBADPARAMETER_BTM);

    if (kdesc == NULL) ERR(eBADPARAMETER_BTM);

    if (kval == NULL) ERR(eBADPARAMETER_BTM);

    if (oid == NULL) ERR(eBADPARAMETER_BTM);    

    /* Error check whether using not supported functionality by EduBtM */
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }
    
    // B+ tree index에 새로운 object를 삽입함
    // 1. edubtm_Insert()를 호출해, <object's key, object ID>를 삽입
    edubtm_Insert(catObjForFile, root, kdesc, kval, oid, &lf, &lh, &item, dlPool, dlHead);
    // 2. 만약 root splitting이 발생한 경우 (lh), edubtm_root_insert()를 호출해 처리
    if (lh) edubtm_root_insert(catObjForFile, root, &item); 
    
    return(eNOERROR);
    
}   /* EduBtM_InsertObject() */
```

4. EduBtM_DeleteIndex
```
Four EduBtM_DeleteObject(
    ObjectID *catObjForFile,	/* IN catalog object of B+-tree file */
    PageID   *root,		/* IN root Page IDentifier */
    KeyDesc  *kdesc,		/* IN a key descriptor */
    KeyValue *kval,		/* IN key value */
    ObjectID *oid,		/* IN Object IDentifier */
    Pool     *dlPool,		/* INOUT pool of dealloc list elements */
    DeallocListElem *dlHead) /* INOUT head of the dealloc list */
{
    int		i;
    Four    e;			/* error number */
    Boolean lf;			/* flag for merging */
    Boolean lh;			/* flag for splitting */
    InternalItem item;		/* Internal item */
    SlottedPage *catPage;	/* buffer page containing the catalog object */
    sm_CatOverlayForBtree *catEntry; /* pointer to Btree file catalog information */
    PhysicalFileID pFid;        /* B+-tree file's FileID */


    /*@ check parameters */
    if (catObjForFile == NULL) ERR(eBADPARAMETER_BTM);

    if (root == NULL) ERR(eBADPARAMETER_BTM);

    if (kdesc == NULL) ERR(eBADPARAMETER_BTM);

    if (kval == NULL) ERR(eBADPARAMETER_BTM);

    if (oid == NULL) ERR(eBADPARAMETER_BTM);
    
    if (dlPool == NULL || dlHead == NULL) ERR(eBADPARAMETER_BTM);

    /* Error check whether using not supported functionality by EduBtM */
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }


	/* Delete following 3 lines before implement this function */
	//printf("and delete operation has not been implemented yet.\n");
	//return(eNOTSUPPORTED_EDUBTM);

    // B+ tree index에서 object를 삭제한다.
    // 1) edubtm_Delete()를 호출해 <object key, object ID>를 B+ tree index에서 제거한다.
    edubtm_Delete(catObjForFile, root, kdesc, kval, oid, &lf, &lh, &item, dlPool, dlHead);
    // 2) underflow가 발생한 경우, btm_root_delete()를 호출해 해결한다.
    if (lf) btm_root_delete(&pFid, root, dlPool, dlHead);
    // 3) root page에서 split이 발생한 경우, edubtm_root_insert()를 호출해 처리한다. 
    else if (lh) edubtm_root_insert(catObjForFile, root, &item);

    BfM_SetDirty(root, PAGE_BUF);
    
    return(eNOERROR);
    
}   /* EduBtM_DeleteObject() */
```

5. EduBtM_Fetch
```
Four EduBtM_Fetch(
    PageID   *root,		/* IN The current root of the subtree */
    KeyDesc  *kdesc,		/* IN Btree key descriptor */
    KeyValue *startKval,	/* IN key value of start condition */
    Four     startCompOp,	/* IN comparison operator of start condition */
    KeyValue *stopKval,		/* IN key value of stop condition */
    Four     stopCompOp,	/* IN comparison operator of stop condition */
    BtreeCursor *cursor)	/* OUT Btree Cursor */
{
    int i;
    Four e;		   /* error number */

    
    if (root == NULL) ERR(eBADPARAMETER_BTM);

    /* Error check whether using not supported functionality by EduBtM */
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }

    // B+ tree index에서 검색 조건을 만족하는 object를 찾아 cursor를 반환한다.

    // 1) startCompOp가 SM_BOF일 경우: 가장 작은 object를 찾아 반환한다.
    if (startCompOp == SM_BOF) e = edubtm_FirstObject(root, kdesc, stopKval, stopCompOp, cursor);
    // 2) startCompOp가 SM_EOF일 경우: 가장 큰 object를 찾아 반환한다.
    else if (startCompOp == SM_EOF) e = edubtm_LastObject(root, kdesc, stopKval, stopCompOp, cursor);
    // 3) 이외의 경우, edubtm_fetch()를 호출해 검색 조건을 만족하는 첫 <object key, object ID>를 찾아 leaf index entry를 반환한다.
    else e = edubtm_Fetch(root, kdesc, startKval, startCompOp, stopKval, stopCompOp, cursor);
    
    // 디버깅용
    if (e < 0) ERR(e);
    return(eNOERROR);

} /* EduBtM_Fetch() */
```

6. EduBtM_FetchNext
```
Four EduBtM_FetchNext(
    PageID                      *root,          /* IN root page's PageID */
    KeyDesc                     *kdesc,         /* IN key descriptor */
    KeyValue                    *kval,          /* IN key value of stop condition */
    Four                        compOp,         /* IN comparison operator of stop condition */
    BtreeCursor                 *current,       /* IN current B+ tree cursor */
    BtreeCursor                 *next)          /* OUT next B+ tree cursor */
{
    int							i;
    Four                        e;              /* error number */
    Four                        cmp;            /* comparison result */
    Two                         slotNo;         /* slot no. of a leaf page */
    Two                         oidArrayElemNo; /* element no. of the array of ObjectIDs */
    Two                         alignedKlen;    /* aligned length of key length */
    PageID                      overflow;       /* temporary PageID of an overflow page */
    Boolean                     found;          /* search result */
    ObjectID                    *oidArray;      /* array of ObjectIDs */
    BtreeLeaf                   *apage;         /* pointer to a buffer holding a leaf page */
    BtreeOverflow               *opage;         /* pointer to a buffer holding an overflow page */
    btm_LeafEntry               *entry;         /* pointer to a leaf entry */
    BtreeCursor                 tCursor;        /* a temporary Btree cursor */
  
    
    /*@ check parameter */
    if (root == NULL || kdesc == NULL || kval == NULL || current == NULL || next == NULL)
	ERR(eBADPARAMETER_BTM);
    
    /* Is the current cursor valid? */
    if (current->flag != CURSOR_ON && current->flag != CURSOR_EOS)
		ERR(eBADCURSOR);
    
    if (current->flag == CURSOR_EOS) return(eNOERROR);
    
    /* Error check whether using not supported functionality by EduBtM */
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }

    // B+ tree index에서 검색 조건을 만족하는 현재 object의 다음 object를 반환한다.
    edubtm_FetchNext(kdesc, kval, compOp, current, next);
    
    return(eNOERROR);
    
} /* EduBtM_FetchNext() */
```

## Low Level
1. edubtm_InitLeaf
```
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
```

2. edubtm_InitInternal
```
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
```

3. edubtm_FreePages
```
Four edubtm_FreePages(
    PhysicalFileID      *pFid,          /* IN FileID of the Btree file */
    PageID              *curPid,        /* IN The PageID to be freed */
    Pool                *dlPool,        /* INOUT pool of dealloc list elements */
    DeallocListElem     *dlHead)        /* INOUT head of the dealloc list */
{
    Four                e;              /* error number */
    Two                 i;              /* index */
    Two                 alignedKlen;    /* aligned length of the key length */
    PageID              tPid;           /* a temporary PageID */
    PageID              ovPid;          /* a temporary PageID of an overflow page */
    BtreePage           *apage;         /* a page pointer */
    BtreeOverflow       *opage;         /* page pointer to a buffer holding an overflow page */
    Two                 iEntryOffset;   /* starting offset of an internal entry */
    Two                 lEntryOffset;   /* starting offset of a leaf entry */
    btm_InternalEntry   *iEntry;        /* an internal entry */
    btm_LeafEntry       *lEntry;        /* a leaf entry */
    DeallocListElem     *dlElem;        /* an element of dealloc list */

    // Deallocate a B+ tree index page
    // 관련 함수: BfM_GetNewTrain, BfM_FreeTrain, BfM_SetDirty, Util_getElementFromPool

    // 1. 재귀적으로 edubtm_FreePages()를 호출해, 모든 child page가 deallocation될 수 있게 함
    BfM_GetNewTrain(curPid, (char**)&apage, PAGE_BUF);

    if (apage->any.hdr.type & INTERNAL) {
        BtreeInternal* cur = &(apage->bi);
        MAKE_PAGEID(tPid, curPid->volNo, cur->hdr.p0);
        edubtm_FreePages(pFid, &tPid, dlPool, dlHead);

        for (i = 0; i<cur->hdr.nSlots; i++) {
            iEntryOffset = apage->bi.slot[-i];
            iEntry = (btm_InternalEntry*)&(cur->data[iEntryOffset]);

            MAKE_PAGEID(tPid, curPid->volNo, iEntry->spid);
            edubtm_FreePages(pFid, &tPid, dlPool, dlHead);
        }
    }

    // 2. page를 deallocation 시킨다.
    apage->any.hdr.type = FREEPAGE;
    // Deallocation element를 받아옴.
    Util_getElementFromPool(dlPool, &dlElem);
    // Deallocation될 page의 정보를 dlElem에 저장
    dlElem->type = DL_PAGE;
    dlElem->elem.pid = *curPid;
    // Dealloc list의 제일 처음에 element를 삽입
    dlElem->next = dlHead->next;
    dlHead->next = dlElem;

    BfM_SetDirty(curPid, PAGE_BUF);
    BfM_FreeTrain(curPid, PAGE_BUF);

    return(eNOERROR);
    
}   /* edubtm_FreePages() */
```

4. edubtm_Insert
```
Four edubtm_Insert(
    ObjectID                    *catObjForFile,         /* IN catalog object of B+-tree file */
    PageID                      *root,                  /* IN the root of a Btree */
    KeyDesc                     *kdesc,                 /* IN Btree key descriptor */
    KeyValue                    *kval,                  /* IN key value */
    ObjectID                    *oid,                   /* IN ObjectID which will be inserted */
    Boolean                     *f,                     /* OUT whether it is merged by creating a new overflow page */
    Boolean                     *h,                     /* OUT whether it is splitted */
    InternalItem                *item,                  /* OUT Internal Item which will be inserted */
                                                        /*     into its parent when 'h' is TRUE */
    Pool                        *dlPool,                /* INOUT pool of dealloc list */
    DeallocListElem             *dlHead)                /* INOUT head of the dealloc list */
{
    Four                        e;                      /* error number */
    Boolean                     lh;                     /* local 'h' */
    Boolean                     lf;                     /* local 'f' */
    Two                         idx;                    /* index for the given key value */
    PageID                      newPid;                 /* a new PageID */
    KeyValue                    tKey;                   /* a temporary key */
    InternalItem                litem;                  /* a local internal item */
    BtreePage                   *apage;                 /* a pointer to the root page */
    btm_InternalEntry           *iEntry;                /* an internal entry */
    Two                         iEntryOffset;           /* starting offset of an internal entry */
    SlottedPage                 *catPage;               /* buffer page containing the catalog object */
    sm_CatOverlayForBtree       *catEntry;              /* pointer to Btree file catalog information */
    PhysicalFileID              pFid;                   /* B+-tree file's FileID */


    /* Error check whether using not supported functionality by EduBtM */
    int i;
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }

    // 1. parameter로 주어진 page를 root page로 하는 B+ tree index에 대해
    // <object key, object ID>를 삽입하고, root split이 발생한 경우, 새 page를 가리키는 internal index entry를 반환

    *h = FALSE;
    *f = FALSE;

    BfM_GetTrain(root, &apage, PAGE_BUF);

    // Case 1. Root page가 INTERNAL page이다.
    if (apage->any.hdr.type & INTERNAL) {
        // 1) 새 <object key, object ID>를 삽입할 leaf page를 찾기 위해 binary search를 수행
        edubtm_BinarySearchInternal(apage, kdesc, kval, &idx);

        // 2) 결정된 자식 page를 root로 하는 B+ subtree에 새로운 <object key, object ID>를 삽입하기 위해
        //    edubtm_Insert()를 재귀적으로 호출
        //    만약 
        iEntryOffset = apage->bi.slot[-idx];
        iEntry = (btm_InternalEntry*) &apage->bi.data[iEntryOffset];

        if (idx < 0) MAKE_PAGEID(newPid, root->volNo, apage->bi.hdr.p0);
        else MAKE_PAGEID(newPid, root->volNo, iEntry->spid);

        lh = FALSE;
        lf = FALSE;

        edubtm_Insert(catObjForFile, &newPid, kdesc, kval, oid, &lf, &lh, &litem, dlPool, dlHead);
        // 3) 자식 page에서 split이 발생한 경우, split으로 생성된 . 새 page를 가리키는 index entry를 반환
        // 4) 파라미터로 주어진 root page에서 split이 발새앟면, split으로 생긴 새 page를 가리키는 index entry를 반환
        if (lh) {
            tKey.len = litem.klen;
            memcpy(tKey.val, litem.kval, tKey.len);
            edubtm_BinarySearchInternal(&(apage->bi), kdesc, &tKey, &idx);
            edubtm_InsertInternal(catObjForFile, &(apage->bi), &litem, idx, h, item);
        }
    }
    // Case 2. Root page가 LEAF page이다.
    else if (apage->any.hdr.type & LEAF) {
        //edubtm_InsertLeaf()를 호출한다. split이 일어나면 new page에 대한 internal index entry를 return
        edubtm_InsertLeaf(catObjForFile, root, &(apage->bl), kdesc, kval, oid, f, h, item);
    }

    BfM_SetDirty(root, PAGE_BUF);
    BfM_FreeTrain(root, PAGE_BUF);
    
    return(eNOERROR);
    
}   /* edubtm_Insert() */
```

5. edubtm_InsertLeaf
```
Four edubtm_InsertLeaf(
    ObjectID                    *catObjForFile, /* IN catalog object of B+-tree file */
    PageID                      *pid,           /* IN PageID of Leag Page */
    BtreeLeaf                   *page,          /* INOUT pointer to buffer page of Leaf page */
    KeyDesc                     *kdesc,         /* IN Btree key descriptor */
    KeyValue                    *kval,          /* IN key value */
    ObjectID                    *oid,           /* IN ObjectID which will be inserted */
    Boolean                     *f,             /* OUT whether it is merged by creating */
                                                /*     a new overflow page */
    Boolean                     *h,             /* OUT whether it is splitted */
    InternalItem                *item)          /* OUT Internal Item which will be inserted */
                                                /*     into its parent when 'h' is TRUE */
{
    Four                        e;              /* error number */
    Two                         i;
    Two                         idx;            /* index for the given key value */
    LeafItem                    leaf;           /* a Leaf Item */
    Boolean                     found;          /* search result */
    btm_LeafEntry               *entry;         /* an entry in a leaf page */
    Two                         entryOffset;    /* start position of an entry */
    Two                         alignedKlen;    /* aligned length of the key length */
    PageID                      ovPid;          /* PageID of an overflow page */
    Two                         entryLen;       /* length of an entry */
    ObjectID                    *oidArray;      /* an array of ObjectIDs */
    Two                         oidArrayElemNo; /* an index for the ObjectID array */


    /* Error check whether using not supported functionality by EduBtM */
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }

    
    /*@ Initially the flags are FALSE */
    *h = *f = FALSE;
    
    // Leaf page에 새 index entry를 삽입한다.
    // split이 발생하면, split으로 생긴 새 leaf page를 가리키는 index entry를 반환한다.

    // 1) 새 index entry를 삽입하기 위한 위치를 결정한다.
    found = edubtm_BinarySearchLeaf(page, kdesc, kval, &idx);

    // 2) 새 index entry를 삽입하기 위해 필요한 free space를 계산
    if (kdesc->kpart[0].type == SM_INT) alignedKlen = ALIGNED_LENGTH(kdesc->kpart[0].length);
    else alignedKlen = ALIGNED_LENGTH(kval->len);

    entryLen = 2 * sizeof(Two) + alignedKlen + OBJECTID_SIZE;

    // index entry 설정
    leaf.oid = *oid;
    leaf.nObjects = 1;
    leaf.klen = kval->len;
    memcpy(leaf.kval, kval->val, leaf.klen);

    // Case 1. Page에 free space가 있다면
    if (BL_FREE(page) >= entryLen + sizeof(Two)) {
        // a) 필요시 page를 compaction함
        if (BL_CFREE(page) < entryLen + sizeof(Two)) edubtm_CompactLeafPage(page, NIL);
        // b) binary search로 찾은 위치에 index entry를 삽입함
        // b-1) 결정된 slotNo를 사용하기 위해 slot array를 재배열
        for (i = page->hdr.nSlots - 1; i >= 1 + idx; i--)
            page->slot[-(i+1)] = page->slot[-i];
        // 결정된 slotNo를 갖는 slot에 새 index entry의 offset을 저장
        entryOffset = page->slot[-(1 + idx)] = page->hdr.free;
        entry = (btm_LeafEntry*)&(page->data[entryOffset]);

        memcpy(entry, &leaf.nObjects, entryLen - OBJECTID_SIZE);
        memcpy(&entry->kval[alignedKlen], &leaf.oid, OBJECTID_SIZE);

        page->hdr.free += entryLen;
        page->hdr.nSlots++;
    }
    // Case 2. Page에 free space가 없다면
    else {
        // a) edubtm_SplitLeaf()을 호출해 page를 나눈다.
        // 새롭게 생긴 internal page를 가리키는 index entry를 반환한다.
        // index entry 설정
        edubtm_SplitLeaf(catObjForFile, pid, page, idx, &leaf, item);
        *h = TRUE;
    }

    return(eNOERROR);
    
} /* edubtm_InsertLeaf() */
```

6. edubtm_InsertInternal
```
Four edubtm_InsertInternal(
    ObjectID            *catObjForFile, /* IN catalog object of B+-tree file */
    BtreeInternal       *page,          /* INOUT Page Pointer */
    InternalItem        *item,          /* IN Iternal item which is inserted */
    Two                 high,           /* IN index in the given page */
    Boolean             *h,             /* OUT whether the given page is splitted */
    InternalItem        *ritem)         /* OUT if the given page is splitted, the internal item may be returned by 'ritem'. */
{
    Four                e;              /* error number */
    Two                 i;              /* index */
    Two                 entryOffset;    /* starting offset of an internal entry */
    Two                 entryLen;       /* length of the new entry */
    btm_InternalEntry   *entry;         /* an internal entry of an internal page */


    
    /*@ Initially the flag are FALSE */
    *h = FALSE;
    
    // Internal page에 새 index entry를 삽입한다.
    // split이 발생하면, split으로 생긴 . 새internal page를 가리키는 index entry를 반환한다.

    // 1) 새 index entry를 삽입하기 위해 필요한 free space를 계산한다.
    entryLen = sizeof(ShortPageID) + sizeof(Two)+ ALIGNED_LENGTH(item->klen);

    // Case 1. Page에 free space가 있다면
    if (BI_FREE(page) >= entryLen + sizeof(Two)) {
        // a) 필요시 page를 compaction함
        if (BI_CFREE(page) < entryLen + sizeof(Two)) edubtm_CompactInternalPage(page, NIL);
        // b) 파라미터로 주어진 slotNo 다음에 index entry를 삽입
        for (i = page->hdr.nSlots - 1; i >= 1 + high; i--)
            page->slot[-(i+1)] = page->slot[-i];

        page->slot[-(high + 1)] = page->hdr.free;

        entry = page->data + page->hdr.free;
        entry->spid = item->spid;
        entry->klen = item->klen;
        memcpy(&entry->kval, item->kval, item->klen);

        page->hdr.free += entryLen;
        page->hdr.nSlots++;
    }
    // Case 2. Page에 free space가 없다면
    else {
        // a) edubtm_SplitInternal()을 호출해 page를 나눈다.
        // 새롭게 생긴 internal page를 가리키는 index entry를 반환한다.
        edubtm_SplitInternal(catObjForFile, page, high, item, ritem);
        *h = TRUE;
    }

    return(eNOERROR);
    
} /* edubtm_InsertInternal() */
```

7. edubtm_SplitLeaf
```
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
    Boolean                     flag = FALSE;
    Boolean                     isTmp;
 
    // Overflow가 발생한 leaf page를 split하여 index entry를 삽입 후
    // 새 leaf page를 가리키는 index entry를 반환한다.

    // 1) 새로운 page를 할당 받고, internal page로 초기화한다.
    btm_AllocPage(catObjForFile, &fpage->hdr.pid, &newPid);
    edubtm_InitLeaf(&newPid, FALSE, FALSE);
    BfM_GetNewTrain(&newPid, &npage, PAGE_BUF);
    // 2) 기존 index entry들과 삽입할 index entry를 key 순으로 정렬한 뒤, 나누어 저장한다.
    // a) 먼저, overflow가 발생한 page (fpage)에 index entry를 50% 정도 채운다.
    sum = 0;
    maxLoop = fpage->hdr.nSlots + 1;   
    j = 0;

    for (i = 0; i < maxLoop && sum < BL_HALF; i++) {
        // 삽입을 진행해야할 object가 fpage에 섞여들어가게 될 경우
        // item이 fpage의 멤버가 되었으므로 flag를 켜주어야한다. 
        if (i == high + 1) {
            entryLen = OBJECTID_SIZE + 2 * sizeof(Two) + ALIGNED_LENGTH(item->klen);
            flag = TRUE;
        }
        else {
            fEntryOffset = fpage->slot[-j];
            fEntry = &fpage->data[fEntryOffset];
            
            entryLen = OBJECTID_SIZE + 2 * sizeof(Two) + ALIGNED_LENGTH(fEntry->klen);
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
            entryLen = OBJECTID_SIZE + 2 * sizeof(Two) + ALIGNED_LENGTH(item->klen);

            nEntry->nObjects = item->nObjects;
            nEntry->klen = item->klen;

            memcpy(nEntry->kval, item->kval, item->klen);
            memcpy(&nEntry->kval[ALIGNED_LENGTH(item->klen)], &item->oid, OBJECTID_SIZE);
        }
        else {
            fEntryOffset = fpage->slot[-j];
            fEntry = &fpage->data[fEntryOffset];

            entryLen = OBJECTID_SIZE + 2 * sizeof(Two) + ALIGNED_LENGTH(fEntry->klen);
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

        entryLen = OBJECTID_SIZE + 2 * sizeof(Two) + ALIGNED_LENGTH(item->klen);

        fpage->hdr.free += OBJECTID_SIZE + 2 * sizeof(Two) + ALIGNED_LENGTH(item->klen);
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
```

8. edubtm_SplitInternal
```
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
    BfM_GetNewTrain(&newPid, &npage, PAGE_BUF);
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
                memcpy(ritem, item, sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(item->klen));
            }
            else {
                fEntryOffset = fpage->slot[-j];
                fEntry = &fpage->data[fEntryOffset];

                entryLen = sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(fEntry->klen);
                memcpy(ritem, fEntry, sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(fEntry->klen));

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
                memcpy(nEntry, item, sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(item->klen));
            }
            else {
                fEntryOffset = fpage->slot[-j];
                fEntry = &fpage->data[fEntryOffset];

                entryLen = sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(fEntry->klen);
                memcpy(nEntry, fEntry, sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(fEntry->klen));

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
```

9. edubtm_root_insert
```
Four edubtm_root_insert(
    ObjectID     *catObjForFile, /* IN catalog object of B+ tree file */
    PageID       *root,		 /* IN root Page IDentifier */
    InternalItem *item)		 /* IN Internal item which will be the unique entry of the new root */
{
    Four      e;		/* error number */
    PageID    newPid;		/* newly allocated page */
    PageID    nextPid;		/* PageID of the next page of root if root is leaf */
    BtreePage *rootPage;	/* pointer to a buffer holding the root page */
    BtreePage *newPage;		/* pointer to a buffer holding the new page */
    BtreeLeaf *nextPage;	/* pointer to a buffer holding next page of root */
    btm_InternalEntry *entry;	/* an internal entry */
    Boolean   isTmp;

    // Root page가 split된 B+ tree index를 위해 새로운 root page를 할당한다.
    // 1. 새로운 page를 할당
    btm_AllocPage(catObjForFile, root, &newPid);
    BfM_GetNewTrain(&newPid, &newPage, PAGE_BUF);
    BfM_GetTrain(root, &rootPage, PAGE_BUF);

    // 2. old root page의 데이터를 새 page에 쓰고, root로 삼는다.
    memcpy(newPage, rootPage, sizeof(BtreePage));
    edubtm_InitInternal(root, TRUE, FALSE);

    newPage->any.hdr.pid = newPid;
    newPage->bi.hdr.pid = newPid;
    newPage->bl.hdr.pid = newPid;

    // 3. 새로운 root page의 자식으로 split된 page와 할당 받은 page를 넣어준다.
    // 3-1) Split으로 생성된 page를 가리키는 internal index entry를 새 root page에 삽입
    entry = rootPage->bi.data + rootPage->bi.hdr.free;
    entry->spid = item->spid;
    entry->klen = item->klen;
    memcpy(entry->kval, item->kval, entry->klen);

    rootPage->bi.slot[-rootPage->bi.hdr.nSlots] = rootPage->bi.hdr.free;
    rootPage->bi.hdr.free += sizeof(ShortPageID) + 2 * sizeof(Two) + ALIGNED_LENGTH(entry->klen);
    rootPage->bi.hdr.nSlots++;

    // 3-2) 새 root page의 p0 값을 할당받은 page로 변경
    rootPage->bi.hdr.p0 = newPid.pageNo;

    // 3-3) 새 root의 child page들이 leaf page라면, 이들을 doulby linked list로 잇는다.
    MAKE_PAGEID(nextPid, root->volNo, entry->spid);
    BfM_GetTrain(&nextPid, &nextPage, PAGE_BUF);

    if ((newPage->any.hdr.type & LEAF) && (nextPage->hdr.type & LEAF)) {
        newPage->bl.hdr.nextPage = nextPid.pageNo;
        nextPage->hdr.prevPage = newPid.pageNo;
    }

    // 4. 마무리
    BfM_SetDirty(&newPid, PAGE_BUF);
    BfM_SetDirty(root, PAGE_BUF);
    BfM_SetDirty(&nextPid, PAGE_BUF);

    BfM_FreeTrain(&newPid, PAGE_BUF);
    BfM_FreeTrain(root, PAGE_BUF);
    BfM_FreeTrain(&nextPid, PAGE_BUF);
    
    return(eNOERROR);
    
} /* edubtm_root_insert() */
```

10. edubtm_Delete
```
Four edubtm_Delete(
    ObjectID                    *catObjForFile, /* IN catalog object of B+ tree file */
    PageID                      *root,          /* IN root page */
    KeyDesc                     *kdesc,         /* IN a key descriptor */
    KeyValue                    *kval,          /* IN key value */
    ObjectID                    *oid,           /* IN Object IDentifier which will be deleted */
    Boolean                     *f,             /* OUT whether the root page is half full */
    Boolean                     *h,             /* OUT TRUE if it is spiltted. */
    InternalItem                *item,          /* OUT The internal item to be returned */
    Pool                        *dlPool,        /* INOUT pool of dealloc list elements */
    DeallocListElem             *dlHead)        /* INOUT head of the dealloc list */
{
    Four                        e;              /* error number */
    Boolean                     lf;             /* TRUE if a page is not half full */
    Boolean                     lh;             /* TRUE if a page is splitted */
    Two                         idx;            /* the index by the binary search */
    PageID                      child;          /* a child page when the root is an internal page */
    KeyValue                    tKey;           /* a temporary key */
    BtreePage                   *rpage;         /* for a root page */
    InternalItem                litem;          /* local internal item */
    btm_InternalEntry           *iEntry;        /* an internal entry */
    SlottedPage                 *catPage;       /* buffer page containing the catalog object */
    sm_CatOverlayForBtree       *catEntry;      /* pointer to Btree file catalog information */
    PhysicalFileID              pFid;           /* B+-tree file's FileID */
  

    /* Error check whether using not supported functionality by EduBtM */
	int i;
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }
        
    *h = *f = FALSE;
    
    
    /* Delete following 2 lines before implement this function */
    //printf("and delete operation has not been implemented yet.\n");
    
    // 파라미터로 주어진 page를 root로 하는 B+ tree index에서 <object key, object ID>를 삭제함
    // 0) B+ tree를 얻어옴
    BfM_GetTrain(catObjForFile, &catPage, PAGE_BUF);
    GET_PTR_TO_CATENTRY_FOR_BTREE(catObjForFile, catPage, catEntry);
    MAKE_PAGEID(pFid, catEntry->fid.volNo, catEntry->firstPage);
    BfM_GetTrain(root, &rpage, PAGE_BUF);

    // Case 1. root page가 internal page
    if (rpage->any.hdr.type & INTERNAL) {
        // 1) 삭제할 pair가 저장된 leaf page를 찾기 위해 탐색을 진행
        edubtm_BinarySearchInternal(&rpage->bi, kdesc, kval, &idx);
        if (idx < 0) MAKE_PAGEID(child, root->volNo, rpage->bi.hdr.p0);
        else {
            iEntry = &rpage->bi.data[rpage->bi.slot[-idx]];
            MAKE_PAGEID(child, root->volNo, iEntry->spid);
        }

        // 2) B+ subtree에서 <object key, object ID>를 삭제하기 위해 재귀적으로 edubtm_Delete()를 호출
        edubtm_Delete(catObjForFile, &child, kdesc, kval, oid, &lf, &lh, &litem, dlPool, dlHead);

        // 3) underflow가 발생한 경우 btm_Unserflow()로 처리함.
        if (lf) {
            btm_Underflow(&pFid, rpage, &child, idx, f, &lh, &litem, dlPool, dlHead);
            BfM_SetDirty(root, PAGE_BUF);
            // a) underfloow가 발생한 자식 page의 부모 page에서 overflow가 발생한 경우,
            // edubtm_InsertInternal()을 호출한다.
            if (lh) {
                memcpy(&tKey, &litem.klen, sizeof(KeyValue));
                edubtm_BinarySearchInternal(rpage, kdesc, &tKey, &idx);
                edubtm_InsertInternal(catObjForFile, rpage, &litem, idx, h, item);
            }
        }
    }
    // Case 2. root page가 leaf page
    else if (rpage->any.hdr.type & LEAF) {
        // edubfm_DeleteLeaf()를 호출해 <object key, object ID>를 삭제함
        edubtm_DeleteLeaf(&pFid, root, rpage, kdesc, kval, oid, f, h, item, dlPool, dlHead);
    }

    BfM_FreeTrain(catObjForFile, PAGE_BUF);
    BfM_FreeTrain(root, PAGE_BUF);

    return(eNOERROR);
    
}   /* edubtm_Delete() */
```

11. edubtm_DeleteLeaf
```
Four edubtm_DeleteLeaf(
    PhysicalFileID              *pFid,          /* IN FileID of the Btree file */
    PageID                      *pid,           /* IN PageID of the leaf page */
    BtreeLeaf                   *apage,         /* INOUT buffer for the Leaf Page */
    KeyDesc                     *kdesc,         /* IN a key descriptor */
    KeyValue                    *kval,          /* IN key value */
    ObjectID                    *oid,           /* IN ObjectID which will be deleted */
    Boolean                     *f,             /* OUT whether the root page is half full */
    Boolean                     *h,             /* OUT TRUE if it is spiltted. */
    InternalItem                *item,          /* OUT The internal item to be returned */
    Pool                        *dlPool,        /* INOUT pool of dealloc list elements */
    DeallocListElem             *dlHead)        /* INOUT head of a dealloc list */
{
    Four                        e;              /* error number */
    Two                         i;              /* index */
    Two                         of;             /* # of ObjectIDs of an overflow page when less than 1/4 */
    Two                         idx;            /* the index by the binary search */
    ObjectID                    tOid;           /* a Object IDentifier */
    BtreeOverflow               *opage;         /* for a overflow page */
    Boolean                     found;          /* Search Result */
    Two                         lEntryOffset;   /* starting offset of a leaf entry */
    btm_LeafEntry               *lEntry;        /* an entry in leaf page */
    ObjectID                    *oidArray;      /* start position of the ObjectID array */
    Two                         oidArrayElemNo; /* element number in the ObjectIDs array */
    Two                         entryLen;       /* length of the old leaf entry */
    Two                         newLen;         /* length of the new leaf entry */
    Two                         alignedKlen;    /* aligned length of the key length */
    PageID                      ovPid;          /* overflow page's PageID */
    DeallocListElem             *dlElem;        /* an element of the dealloc list */


    /* Error check whether using not supported functionality by EduBtM */
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }


    /* Delete following 2 lines before implement this function */
    //printf("and delete operation has not been implemented yet.\n");

    // Leaf page에서 <object key, object ID> pair를 삭제한다.

    // 1) 삭제할 <object key, object ID> pair가 저장된 index entry의 slot을 삭제한다.
    // slot array는 compaction을 시킨다.
    found = edubtm_BinarySearchLeaf(apage, kdesc, kval, &idx);
    if (found) {
        lEntry = &apage->data[apage->slot[-idx]];
        lEntryOffset = apage->slot[-idx];
        
        entryLen = OBJECTID_SIZE + 2 * sizeof(Two) + ALIGNED_LENGTH(lEntry->klen);

        memcpy(&tOid, &lEntry->kval[ALIGNED_LENGTH(lEntry->klen)], OBJECTID_SIZE);

        // Compaction
        if (btm_ObjectIdComp(oid, &tOid) == EQUAL) {
            for (i = idx; i < apage->hdr.nSlots - 1; i++) {
                apage->slot[-i] = apage->slot[-(i+1)];
            }

            if (lEntryOffset + entryLen == apage->hdr.free) apage->hdr.free -= entryLen;
            else apage->hdr.unused += entryLen;

            apage->hdr.nSlots--;
        }
    }

    // 3) 만약 leaf page에서 underflow가 발생하면, f를 TRUE로 설정한다.
    if (BL_FREE(apage) > BL_HALF) *f = TRUE;

    BfM_SetDirty(pid, PAGE_BUF);
	      
    return(eNOERROR);
    
} /* edubtm_DeleteLeaf() */
```

12. edubtm_CompactLeafPage
```
void edubtm_CompactLeafPage(
    BtreeLeaf 		*apage,			/* INOUT leaf page to compact */
    Two       		slotNo)			/* IN slot to go to the boundary of free space */
{	
    BtreeLeaf 		tpage;			/* temporay page used to save the given page */
    Two                 apageDataOffset;        /* where the next object is to be moved */
    Two                 len;                    /* length of the leaf entry */
    Two                 i;                      /* index variable */
    btm_LeafEntry 	*entry;			/* an entry in leaf page */
    Two 		alignedKlen;		/* aligned length of the key length */

    // 1) Leaf page의 모든 free space가 하나의 연속된 공간을 만들도록 offset을 재조정
    apageDataOffset = 0;

    // Case 1. 만약 slotNo가 NIL이 아니라면
    if (slotNo != NIL) {
        // slotNo에 대응하는 index entry를 제외한 모든 index entry를 앞에서부터 저장한다.
        for (i = 0; i < apage->hdr.nSlots; i++) {
            if (i != slotNo) {
                entry = apage->data + apage->slot[-i];
                //len = OBJECTID_SIZE + 2 * sizeof(Two) + ALIGNED_LENGTH(entry->klen);
                // overflow?
                
                memcpy(tpage.data + apageDataOffset, entry, OBJECTID_SIZE + 2 * sizeof(Two) + ALIGNED_LENGTH(entry->klen));
                apage->slot[-i] = apageDataOffset;
                apageDataOffset += OBJECTID_SIZE + 2 * sizeof(Two) + ALIGNED_LENGTH(entry->klen);
            }
        }
        // slotNo에 대응되는 index entry는 마지막 index entry로 저장한다.
        entry = apage->data + apage->slot[-slotNo];
        //len = OBJECTID_SIZE + 2 * sizeof(Two) + ALIGNED_LENGTH(entry->klen);
        // overflow?
                
        memcpy(tpage.data + apageDataOffset, entry, OBJECTID_SIZE + 2 * sizeof(Two) + ALIGNED_LENGTH(entry->klen));
        apage->slot[-slotNo] = apageDataOffset;
        apageDataOffset += OBJECTID_SIZE + 2 * sizeof(Two) + ALIGNED_LENGTH(entry->klen);

        memcpy(apage->data, tpage.data, apageDataOffset);
    }
    // Case 2. 만약 slotNo가 NIL이라면
    else {
        // page의 모든 index entry를 앞에서부터 저장한다.
        for (i = 0; i < apage->hdr.nSlots; i++) {
            if (i != slotNo) {
                entry = apage->data + apage->slot[-i];
                //len = OBJECTID_SIZE + 2 * sizeof(Two) + ALIGNED_LENGTH(entry->klen);
                // overflow?

                memcpy(tpage.data + apageDataOffset, entry, OBJECTID_SIZE + 2 * sizeof(Two) + ALIGNED_LENGTH(entry->klen));
                apage->slot[-i] = apageDataOffset;
                apageDataOffset += OBJECTID_SIZE + 2 * sizeof(Two) + ALIGNED_LENGTH(entry->klen);
            }
        }
        memcpy(apage->data, tpage.data, apageDataOffset);
    }

    // 2) Page header 갱신
    apage->hdr.free = apageDataOffset;
    apage->hdr.unused = 0;

} /* edubtm_CompactLeafPage() */
```

13. edubtm_CompactInternalPage
```
void edubtm_CompactInternalPage(
    BtreeInternal       *apage,                 /* INOUT internal page to compact */
    Two                 slotNo)                 /* IN slot to go to the boundary of free space */
{
    BtreeInternal       tpage;                  /* temporay page used to save the given page */
    Two                 apageDataOffset;        /* where the next object is to be moved */
    Two                 len;                    /* length of the leaf entry */
    Two                 i;                      /* index variable */
    btm_InternalEntry   *entry;                 /* an entry in leaf page */

    // 1) Internal page의 모든 free space가 하나의 연속된 공간을 만들도록 offset을 재조정
    apageDataOffset = 0;

    // Case 1. 만약 slotNo가 NIL이 아니라면
    if (slotNo != NIL) {
        // slotNo에 대응하는 index entry를 제외한 모든 index entry를 앞에서부터 저장한다.
        for (i = 0; i < apage->hdr.nSlots; i++) {
            if (i != slotNo) {
                entry = apage->data + apage->slot[-i];
                len = sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(entry->klen);
                
                memcpy(tpage.data + apageDataOffset, entry, len);
                apage->slot[-i] = apageDataOffset;
                apageDataOffset += len;
            }
        }
        // slotNo에 대응되는 index entry는 마지막 index entry로 저장한다.
        entry = apage->data + apage->slot[-slotNo];
        len = sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(entry->klen);
                
        memcpy(tpage.data + apageDataOffset, entry, len);
        apage->slot[-slotNo] = apageDataOffset;
        apageDataOffset += len;

        memcpy(apage->data, tpage.data, apageDataOffset);
    }
    // Case 2. 만약 slotNo가 NIL이라면
    else {
        // page의 모든 index entry를 앞에서부터 저장한다.
        for (i = 0; i < apage->hdr.nSlots; i++) {
            if (i != slotNo) {
                entry = apage->data + apage->slot[-i];
                len = sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(entry->klen);
                
                memcpy(tpage.data + apageDataOffset, entry, len);
                apage->slot[-i] = apageDataOffset;
                apageDataOffset += len;
            }
        }
        memcpy(apage->data, tpage.data, apageDataOffset);
    }

    // 2) Page header 갱신
    apage->hdr.free = apageDataOffset;
    apage->hdr.unused = 0;

} /* edubtm_CompactInternalPage() */
```

14. edubtm_Fetch
```
Four edubtm_Fetch(
    PageID              *root,          /* IN The current root of the subtree */
    KeyDesc             *kdesc,         /* IN Btree key descriptor */
    KeyValue            *startKval,     /* IN key value of start condition */
    Four                startCompOp,    /* IN comparison operator of start condition */
    KeyValue            *stopKval,      /* IN key value of stop condition */
    Four                stopCompOp,     /* IN comparison operator of stop condition */
    BtreeCursor         *cursor)        /* OUT Btree Cursor */
{
    Four                e;              /* error number */
    Four                cmp;            /* result of comparison */
    Two                 idx;            /* index */
    PageID              child;          /* child page when the root is an internla page */
    Two                 alignedKlen;    /* aligned size of the key length */
    BtreePage           *apage;         /* a Page Pointer to the given root */
    BtreeOverflow       *opage;         /* a page pointer if it necessary to access an overflow page */
    Boolean             found;          /* search result */
    PageID              *leafPid;       /* leaf page pointed by the cursor */
    Two                 slotNo;         /* slot pointed by the slot */
    PageID              ovPid;          /* PageID of the overflow page */
    PageNo              ovPageNo;       /* PageNo of the overflow page */
    PageID              prevPid;        /* PageID of the previous page */
    PageID              nextPid;        /* PageID of the next page */
    ObjectID            *oidArray;      /* array of the ObjectIDs */
    Two                 iEntryOffset;   /* starting offset of an internal entry */
    btm_InternalEntry   *iEntry;        /* an internal entry */
    Two                 lEntryOffset;   /* starting offset of a leaf entry */
    btm_LeafEntry       *lEntry;        /* a leaf entry */


    /* Error check whether using not supported functionality by EduBtM */
    int i;
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }

    // 파라미터로 주어진 page를 root page로 하는 B+ tree index에서 검색조건을 만족하는 첫 <object key, object ID>를 반환
    // (검색 시작 key 값과 가장 가까운 key 값)
    BfM_GetTrain(root, &apage, PAGE_BUF);

    // case 1. root page가 internal page
    if (apage->any.hdr.type & INTERNAL) {
        // 1) 방문해야할 child page를 찾는다.
        found = edubtm_BinarySearchInternal(&apage->bi, kdesc, startKval, &idx);
        if (idx < 0) MAKE_PAGEID(child, root->volNo, apage->bi.hdr.p0);
        else {
            iEntry = &(apage->bi.data[apage->bi.slot[-idx]]);
            MAKE_PAGEID(child, root->volNo, iEntry->spid);
        }
        // 2) 재귀적으로 edubtm_Fetch()를 불러, leaf index entry를 탐색한다.
        edubtm_Fetch(&child, kdesc, startKval, startCompOp, stopKval, stopCompOp, cursor);
        BfM_FreeTrain(root, PAGE_BUF);
    }
    // case 2. root page가 leaf page
    else if (apage->any.hdr.type & LEAF) {
        // 1) 검색 조건을 만족하는 첫 <object key, object ID>가 저장된 index entry를 검색함.
        // (SM_EQ, SM_LT, SM_LE, SM_GT, SM_GE 5가지 경우를 처리해주어야함)
        leafPid = root;
        cursor->flag = CURSOR_ON;

        found = edubtm_BinarySearchLeaf(&apage->bl, kdesc, startKval, &slotNo);
        // For SM_EOF, SM_BOF
        idx = slotNo;

        // case 1. SM_EQ 
        if (startCompOp == SM_EQ) {
            if (found == TRUE) idx = slotNo;
            else {
                cursor->flag = CURSOR_EOS;
                BfM_FreeTrain(root, PAGE_BUF);
                return eNOERROR;
            }
        }
        // case 2. SM_LT
        if (startCompOp == SM_LT) {
            if (found == TRUE) idx = slotNo - 1;
            else idx = slotNo;
        }
        // case 3. SM_LE
        if (startCompOp == SM_LE) {
            if (found == TRUE) idx = slotNo;
            else idx = slotNo;
        }
        // case 4. SM_GE
        if (startCompOp == SM_GE) {
            if (found == TRUE) idx = slotNo;
            else idx = slotNo + 1;
        }
        // case 5. SM_GT
        if (startCompOp == SM_GT) {
            if (found == TRUE) idx = slotNo + 1;
            else idx = slotNo + 1;
        }

        // 다음/이전 page에서 원소를 가져와야하는지를 검사
        // case 1. 이전 page에서 마지막 원소를 가지고 와야함
        if (idx < 0) {
            if (apage->bl.hdr.prevPage == NIL) cursor->flag = CURSOR_EOS;
            else {
                MAKE_PAGEID(prevPid, root->volNo, apage->bl.hdr.prevPage);
                leafPid = &prevPid;

                BfM_FreeTrain(root, PAGE_BUF);
                BfM_GetTrain(leafPid, &apage, PAGE_BUF);
                idx = apage->bl.hdr.nSlots - 1;
            }
        }
        // case 2. 다음 page에서 첫 원소를 가지고 와야함
        else if (idx >= apage->bl.hdr.nSlots) {
            if (apage->bl.hdr.nextPage == NIL) cursor->flag = CURSOR_EOS;
            else {
                MAKE_PAGEID(nextPid, root->volNo, apage->bl.hdr.nextPage);
                leafPid = &nextPid;

                BfM_FreeTrain(root, PAGE_BUF);
                BfM_GetTrain(leafPid, &apage, PAGE_BUF);
                idx = 0;
            }
        }

        cursor->leaf = *leafPid;
        cursor->slotNo = idx;

        lEntryOffset = apage->bl.slot[-idx];
        lEntry = &(apage->bl.data[lEntryOffset]);
        alignedKlen = ALIGNED_LENGTH(lEntry->klen);

        memcpy(&cursor->oid, &lEntry->kval + alignedKlen, sizeof(ObjectID));
        memcpy(&cursor->key, &lEntry->klen, sizeof(KeyValue));

        cmp = edubtm_KeyCompare(kdesc, &cursor->key, stopKval);
        if (stopCompOp == SM_LE && (cmp != LESS && cmp != EQUAL)) cursor->flag = CURSOR_EOS;
        else if (stopCompOp == SM_LT && cmp != LESS) cursor->flag = CURSOR_EOS;
        else if (stopCompOp == SM_GE && (cmp != GREAT && cmp != EQUAL)) cursor->flag = CURSOR_EOS;
        else if (stopCompOp == SM_GT && cmp != GREAT) cursor->flag = CURSOR_EOS;

        BfM_FreeTrain(leafPid, PAGE_BUF);
    }

    return(eNOERROR);
} /* edubtm_Fetch() */
```

15. edubtm_FetchNext
```
Four edubtm_FetchNext(
    KeyDesc  		*kdesc,		/* IN key descriptor */
    KeyValue 		*kval,		/* IN key value of stop condition */
    Four     		compOp,		/* IN comparison operator of stop condition */
    BtreeCursor 	*current,	/* IN current cursor */
    BtreeCursor 	*next)		/* OUT next cursor */
{
    Four 		e;		/* error number */
    Four 		cmp;		/* comparison result */
    Two 		alignedKlen;	/* aligned length of a key length */
    PageID 		leaf;		/* temporary PageID of a leaf page */
    PageID 		overflow;	/* temporary PageID of an overflow page */
    ObjectID 		*oidArray;	/* array of ObjectIDs */
    BtreeLeaf 		*apage;		/* pointer to a buffer holding a leaf page */
    BtreeOverflow 	*opage;		/* pointer to a buffer holding an overflow page */
    btm_LeafEntry 	*entry;		/* pointer to a leaf entry */    
    
    
    /* Error check whether using not supported functionality by EduBtM */
    int i;
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }

    // B+ tree index에서 검색 조건을 만족하는 현재 object의 다음 object를 반환한다.
    // 1. 위치해있는 page와 slotNo를 알아낸다
    leaf = current->leaf;
    BfM_GetTrain(&leaf, &apage, PAGE_BUF);

    overflow = leaf;
    next->flag = CURSOR_ON;
    
    // 1) SM_EQ
    if (compOp == SM_EQ) {
        next->flag = CURSOR_EOS;
        BfM_FreeTrain(&leaf, PAGE_BUF);
        return (eNOERROR);
    }
    // 2) SM_LT, SM_LE (+ SM_EOF)
    // 더 작은 entry를 반환한다.
    else if (compOp == SM_LT || compOp == SM_LE || compOp == SM_EOF) {
        next->slotNo = current->slotNo + 1;
    }
    // 3) SM_GT, SM_GE (+ SM_BOF)
    // 더 큰 entry를 반환한다.
    else if (compOp == SM_GE || compOp == SM_GT || compOp == SM_BOF) {
        next->slotNo = current->slotNo - 1;
    }

    if (next->slotNo < 0) {
        if (apage->hdr.prevPage == NIL) next->flag = CURSOR_EOS;
        else {
            BfM_FreeTrain(&leaf, PAGE_BUF);

            MAKE_PAGEID(overflow, leaf.volNo, apage->hdr.prevPage);
            BfM_GetTrain(&overflow, &apage, PAGE_BUF);

            next->slotNo = apage->hdr.nSlots - 1;
        }
    }
    else if (next->slotNo >= apage->hdr.nSlots) {
        if (apage->hdr.nextPage == NIL) next->flag = CURSOR_EOS;
        else {
            BfM_FreeTrain(&leaf, PAGE_BUF);

            MAKE_PAGEID(overflow, leaf.volNo, apage->hdr.nextPage);
            BfM_GetTrain(&overflow, &apage, PAGE_BUF);

            next->slotNo = 0;
        }
    }
    else {
        BfM_FreeTrain(&leaf, PAGE_BUF);
        BfM_GetTrain(&overflow, &apage, PAGE_BUF);
    }
        
    // 2. 해당 정보들을 바탕으로, next entry의 남은 정보들을 기록한다.
    next->leaf = overflow;

    entry = &apage->data[apage->slot[-next->slotNo]];
	alignedKlen = ALIGNED_LENGTH(entry->klen);

	memcpy(&next->oid, &(entry->kval[alignedKlen]), OBJECTID_SIZE);
	memcpy(&next->key, &entry->klen, sizeof(KeyValue));

    cmp = edubtm_KeyCompare(kdesc, &next->key, kval);
	if (compOp == SM_LE && (cmp != LESS && cmp != EQUAL)) next->flag = CURSOR_EOS;
    else if (compOp == SM_LT && cmp != LESS) next->flag = CURSOR_EOS;
    else if (compOp == SM_GE && (cmp != GREAT && cmp != EQUAL)) next->flag = CURSOR_EOS;
    else if (compOp == SM_GT && (cmp != GREAT)) next->flag = CURSOR_EOS;
            
    // 4. 마무리
    BfM_FreeTrain(&overflow, PAGE_BUF);

    return(eNOERROR);
    
} /* edubtm_FetchNext() */
```

16. edubtm_FirstObject
```
Four edubtm_FirstObject(
    PageID  		*root,		/* IN The root of Btree */
    KeyDesc 		*kdesc,		/* IN Btree key descriptor */
    KeyValue 		*stopKval,	/* IN key value of stop condition */
    Four     		stopCompOp,	/* IN comparison operator of stop condition */
    BtreeCursor 	*cursor)	/* OUT The first ObjectID in the Btree */
{
    int			i;
    Four 		e;		/* error */
    Four 		cmp;		/* result of comparison */
    PageID 		curPid;		/* PageID of the current page */
    PageID 		child;		/* PageID of the child page */
    BtreePage 		*apage;		/* a page pointer */
    Two                 lEntryOffset;   /* starting offset of a leaf entry */
    btm_LeafEntry 	*lEntry;	/* a leaf entry */
    Two                 alignedKlen;    /* aligned length of the key length */
    

    if (root == NULL) ERR(eBADPAGE_BTM);

    /* Error check whether using not supported functionality by EduBtM */
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }
    
    // B+ tree index에서 가장 작은 key 값을 가진 leaf index entry를 가리키는 cursor를 반환한다.
    // root부터 시작
    BfM_GetTrain(root, &apage, PAGE_BUF);

    // case 1. INTERNAL PAGE -> 재귀적으로 왼쪽으로 타고들어간다.
    if (apage->any.hdr.type & INTERNAL) {
        MAKE_PAGEID(child, root->volNo, apage->bi.hdr.p0);
        edubtm_FirstObject(&child, kdesc, stopKval, stopCompOp, cursor);
    }
    // case 2. LEAF PAGE -> 안쪽에서 제일 작은 key값을 찾는다.
    else if (apage->any.hdr.type & LEAF) {
        lEntry = &(apage->bl.data[apage->bl.slot[0]]);
        memcpy(&cursor->key, &lEntry->klen, sizeof(KeyValue));

        // 만약 검색 종료 key 값 stopKval이 첫 object의 key 값보다 작거나, key 값이 같으나 종료 연산이 SM_LT일 때 CURSOR_EOS를 반환한다.
        cmp = edubtm_KeyCompare(kdesc, stopKval, &cursor->key);
        if (cmp == LESS || (cmp == EQUAL && stopCompOp == SM_LT)) cursor->flag = CURSOR_EOS;
        else {
            cursor->flag = CURSOR_ON;
            cursor->leaf = *root;
            cursor->slotNo = 0;
            cursor->oid = ((ObjectID*)&lEntry->kval[ALIGNED_LENGTH(cursor->key.len)])[0];
        }
    }

    // 마무리
    BfM_FreeTrain(root, PAGE_BUF);

    return(eNOERROR);
    
} /* edubtm_FirstObject() */
```

17. edubtm_LastObject
```
Four edubtm_LastObject(
    PageID   		*root,		/* IN the root of Btree */
    KeyDesc  		*kdesc,		/* IN key descriptor */
    KeyValue 		*stopKval,	/* IN key value of stop condition */
    Four     		stopCompOp,	/* IN comparison operator of stop condition */
    BtreeCursor 	*cursor)	/* OUT the last BtreeCursor to be returned */
{
    int			i;
    Four 		e;		/* error number */
    Four 		cmp;		/* result of comparison */
    BtreePage 		*apage;		/* pointer to the buffer holding current page */
    BtreeOverflow 	*opage;		/* pointer to the buffer holding overflow page */
    PageID 		curPid;		/* PageID of the current page */
    PageID 		child;		/* PageID of the child page */
    PageID 		ovPid;		/* PageID of the current overflow page */
    PageID 		nextOvPid;	/* PageID of the next overflow page */
    Two 		lEntryOffset;	/* starting offset of a leaf entry */
    Two 		iEntryOffset;	/* starting offset of an internal entry */
    btm_LeafEntry 	*lEntry;	/* a leaf entry */
    btm_InternalEntry 	*iEntry;	/* an internal entry */
    Four 		alignedKlen;	/* aligned length of the key length */
        

    if (root == NULL) ERR(eBADPAGE_BTM);

    /* Error check whether using not supported functionality by EduBtM */
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }
    
    // B+ tree index에서 가장 큰 key 값을 가진 leaf index entry를 가리키는 cursor를 반환한다.
    // root부터 시작
    BfM_GetTrain(root, &apage, PAGE_BUF);

    // case 1. INTERNAL PAGE -> 재귀적으로 왼쪽으로 타고들어간다.
    if (apage->any.hdr.type & INTERNAL) {
        MAKE_PAGEID(child, root->volNo, apage->bi.hdr.p0);
        edubtm_LastObject(&child, kdesc, stopKval, stopCompOp, cursor);
    }
    // case 2. LEAF PAGE -> 안쪽에서 제일 작은 key값을 찾는다.
    else if (apage->any.hdr.type & LEAF) {
        // 1) 만약 nextPage가 있다면, doubly linked list를 따라간다.
        if (apage->bl.hdr.nextPage != NIL) {
            MAKE_PAGEID(curPid, root->volNo, apage->bl.hdr.nextPage);
            edubtm_LastObject(&curPid, kdesc, stopKval, stopCompOp, cursor);
        }
        // 2) 마지막 page를 찾았다면, 제일 key 값이 큰 index entry를 찾는다.
        else {
            lEntry = apage->bl.data + apage->bl.slot[-(apage->bl.hdr.nSlots - 1)];
            memcpy(&cursor->key, &lEntry->klen, sizeof(KeyValue));
            
            // 만약 검색 종료 key 값 stopKval이 첫 object의 key 값보다 크거나, key 값이 같으나 종료 연산이 SM_GT일 때 CURSOR_EOS를 반환한다.
            cmp = edubtm_KeyCompare(kdesc, stopKval, &cursor->key);
            if (cmp == GREAT || (cmp == EQUAL && stopCompOp == SM_GT)) cursor->flag = CURSOR_EOS;
            else {
                cursor->flag = CURSOR_ON;
                cursor->leaf = *root;
                cursor->slotNo = apage->bl.hdr.nSlots - 1;
                cursor->oid = ((ObjectID*)&lEntry->kval[ALIGNED_LENGTH(cursor->key.len)])[0];
            }
        }
    }

    // 마무리
    BfM_FreeTrain(root, PAGE_BUF);

    return(eNOERROR);
    
} /* edubtm_LastObject() */
```

18. edubtm_BinarySearchLeaf
```
Boolean edubtm_BinarySearchLeaf(
    BtreeLeaf 		*lpage,		/* IN Page Pointer to a leaf page */
    KeyDesc   		*kdesc,		/* IN key descriptor */
    KeyValue  		*kval,		/* IN key value */
    Two       		*idx)		/* OUT index to be returned */
{
    Two  		lo;		/* low index */
    Two  		mid;		/* mid index */
    Two  		hi;		/* high index */
    Four 		cmp;		/* result of comparison */
    btm_LeafEntry 	*entry;		/* a leaf entry */


    /* Error check whether using not supported functionality by EduBtM */
    int i;
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }

    // Leaf page 중 파라미터로 주어진 kval 이하의 key값을 갖는 index entry의 slot 번호를 반환
    // 3) kval이 주어진 page의 index entry 모두보다 작다면..
    if (lpage->hdr.nSlots == 0) {
        *idx = -1;
        return FALSE;
    }

    lo = 0, hi = lpage->hdr.nSlots - 1;
    while(lo <= hi) {
        mid = (lo + hi) >> 1;
        entry = &(lpage->data[lpage->slot[-mid]]);
        
        // key 값을 비교
        cmp = edubtm_KeyCompare(kdesc, kval, &entry->klen);

        if (cmp == LESS) hi = mid - 1;
        else if (cmp == GREAT) lo = mid + 1;
        else if (cmp == EQUAL) break;
    }

    if (cmp == EQUAL) {
        *idx = mid;
        return TRUE;
    }
    else {
        while(cmp != GREAT && mid >= 0) {
            mid--;
            entry = &(lpage->data[lpage->slot[-mid]]);
            cmp = edubtm_KeyCompare(kdesc, kval, &entry->klen);
        }
    }
    
    // 2) 같은 key를 가진 index entry가 없다면 kval 이하의 key 값을 갖는 index entry 중
    // 가장 큰 key 값을 가진 index entry와 FALSE를 반환
    *idx = hi;
    return FALSE;

} /* edubtm_BinarySearchLeaf() */
```

19. edubtm_BinarySearchInternal
```
Boolean edubtm_BinarySearchInternal(
    BtreeInternal 	*ipage,		/* IN Page Pointer to an internal page */
    KeyDesc       	*kdesc,		/* IN key descriptor */
    KeyValue      	*kval,		/* IN key value */
    Two          	*idx)		/* OUT index to be returned */
{
    Two  		lo;		/* low index */
    Two  		mid;		/* mid index */
    Two  		hi;		/* high index */
    Four 		cmp;		/* result of comparison */
    btm_InternalEntry 	*entry;	/* an internal entry */

    
    /* Error check whether using not supported functionality by EduBtM */
    int i;
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }

    // Internal page들 중에서 파라미터로 주어진 kval 이하의 key 값을 갖는 index entry를 검색해 slot 번호를 반환한다.
    // 3) kval이 주어진 page의 index entry 모두보다 작다면..
    if (ipage->hdr.nSlots == 0) {
        *idx = -1;
        return FALSE;
    }

    lo = 0, hi = ipage->hdr.nSlots - 1;
    while(lo <= hi) {
        mid = (lo + hi) >> 1;
        entry = &(ipage->data[ipage->slot[-mid]]);
        
        // key 값을 비교
        cmp = edubtm_KeyCompare(kdesc, kval, &entry->klen);

        if (cmp == LESS) hi = mid - 1;
        else if (cmp == GREAT) lo = mid + 1;
        else if (cmp == EQUAL) break;
    }

    if (cmp == EQUAL) {
        *idx = mid;
        return TRUE;
    }
    else {
        while(cmp != GREAT && mid >= 0) {
            mid--;
            entry = &(ipage->data[ipage->slot[-mid]]);
            cmp = edubtm_KeyCompare(kdesc, kval, &entry->klen);
        }
    }

    // 2) 같은 key를 가진 index entry가 없다면 kval 이하의 key 값을 갖는 index entry 중
    // 가장 큰 key 값을 가진 index entry와 FALSE를 반환
    *idx = mid;
    return FALSE;
} /* edubtm_BinarySearchInternal() */
```

20. edubtm_KeyCompare
```
Four edubtm_KeyCompare(
    KeyDesc                     *kdesc,		/* IN key descriptor for key1 and key2 */
    KeyValue                    *key1,		/* IN the first key value */
    KeyValue                    *key2)		/* IN the second key value */
{
    register unsigned char      *left;          /* left key value */
    register unsigned char      *right;         /* right key value */
    Two                         i;              /* index for # of key parts */
    Two                         j1, j2;              /* temporary variable */
    Two                         kpartSize;      /* size of the current kpart */
    Two                         len1, len2;	/* string length */
    Two_Invariable              s1, s2;         /* 2-byte short values */
    Four_Invariable             i1, i2;         /* 4-byte int values */
    Four_Invariable             l1, l2;         /* 4-byte long values */
    Eight_Invariable            ll1, ll2;       /* 8-byte long long values */
    float                       f1, f2;         /* float values */
    double                      d1, d2;		/* double values */
    PageID                      pid1, pid2;	/* PageID values */
    OID                         oid1, oid2;     /* OID values */
    

    /* Error check whether using not supported functionality by EduBtM */
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }

    // 파라미터로 주어진 key1, key2의 대소를 비교하고 결과를 반환한다.
    // 같을 경우 EQUAL, key1이 더 클 경우 GREAT, key1이 작을 경우 LESS
    left = key1->val;
    right = key2->val;

    j1 = 0, j2 = 0;
    for (int i = 0; i < kdesc->nparts; i++) {
        // case 1. INT
        if (kdesc->kpart[i].type == SM_INT) {
            kpartSize = kdesc->kpart[i].length;
            memcpy(&i1, &left[j1], kpartSize);
            memcpy(&i2, &right[j2], kpartSize);

            if (i1 > i2) return GREAT;
            else if (i1 < i2) return LESS;

            j1 += kpartSize, j2 += kpartSize;
        }
        // case 2. VARSTRING
        else if (kdesc->kpart[i].type == SM_VARSTRING) {
            memcpy(&len1, &left[j1], sizeof(Two));
            memcpy(&len2, &right[j2], sizeof(Two));

            if (strcmp(&left[j1+sizeof(Two)], &right[j1+sizeof(Two)]) > 0) return GREAT;
			else if (strcmp(&left[j1+sizeof(Two)], &right[j1+sizeof(Two)]) < 0) return LESS;
			
			j1 += len1 + sizeof(Two);
			j2 += len2 + sizeof(Two);
        }
        else break;
    }
    
    return EQUAL;
    
}   /* edubtm_KeyCompare() */
```