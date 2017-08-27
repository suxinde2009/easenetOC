//=====================================================================
//
// inetsim.h - network simulator for IP-layer
//
// NOTE:
// for more information, please see the readme file
//
//=====================================================================

#ifndef __INETSIM_H__
#define __INETSIM_H__


/*====================================================================*/
/* QUEUE DEFINITION                                                   */
/*====================================================================*/
#ifndef __IQUEUE_DEF__
#define __IQUEUE_DEF__

struct IQUEUEHEAD {
	struct IQUEUEHEAD *next, *prev;
};

typedef struct IQUEUEHEAD iqueue_head;


/*--------------------------------------------------------------------*/
/* queue init                                                         */
/*--------------------------------------------------------------------*/
#define IQUEUE_HEAD_INIT(name) { &(name), &(name) }
#define IQUEUE_HEAD(name) \
	struct IQUEUEHEAD name = IQUEUE_HEAD_INIT(name)

#define IQUEUE_INIT(ptr) ( \
	(ptr)->next = (ptr), (ptr)->prev = (ptr))

#define IOFFSETOF(TYPE, MEMBER) ((unsigned long) &((TYPE *)0)->MEMBER)

#define ICONTAINEROF(ptr, type, member) ( \
		(type*)( ((char*)((type*)ptr)) - IOFFSETOF(type, member)) )

#define IQUEUE_ENTRY(ptr, type, member) ICONTAINEROF(ptr, type, member)


/*--------------------------------------------------------------------*/
/* queue operation                                                    */
/*--------------------------------------------------------------------*/
#define IQUEUE_ADD(node, head) ( \
	(node)->prev = (head), (node)->next = (head)->next, \
	(head)->next->prev = (node), (head)->next = (node))

#define IQUEUE_ADD_TAIL(node, head) ( \
	(node)->prev = (head)->prev, (node)->next = (head), \
	(head)->prev->next = (node), (head)->prev = (node))

#define IQUEUE_DEL_BETWEEN(p, n) ((n)->prev = (p), (p)->next = (n))

#define IQUEUE_DEL(entry) (\
	(entry)->next->prev = (entry)->prev, \
	(entry)->prev->next = (entry)->next, \
	(entry)->next = 0, (entry)->prev = 0)

#define IQUEUE_DEL_INIT(entry) do { \
	IQUEUE_DEL(entry); IQUEUE_INIT(entry); } while (0);

#define IQUEUE_IS_EMPTY(entry) ((entry) == (entry)->next)

#define iqueue_init		IQUEUE_INIT
#define iqueue_entry	IQUEUE_ENTRY
#define iqueue_add		IQUEUE_ADD
#define iqueue_add_tail	IQUEUE_ADD_TAIL
#define iqueue_del		IQUEUE_DEL
#define iqueue_del_init	IQUEUE_DEL_INIT
#define iqueue_is_empty IQUEUE_IS_EMPTY

#define IQUEUE_FOREACH(iterator, head, TYPE, MEMBER) \
	for ((iterator) = iqueue_entry((head)->next, TYPE, MEMBER); \
		&((iterator)->MEMBER) != (head); \
		(iterator) = iqueue_entry((iterator)->MEMBER.next, TYPE, MEMBER))

#define iqueue_foreach(iterator, head, TYPE, MEMBER) \
	IQUEUE_FOREACH(iterator, head, TYPE, MEMBER)


#ifdef _MSC_VER
#pragma warning(disable:4311)
#pragma warning(disable:4312)
#pragma warning(disable:4996)
#endif

#endif



/*====================================================================*/
/* GLOBAL DEFINITION                                                  */
/*====================================================================*/

// ģ�����ݰ�
struct ISIMPACKET
{
	struct IQUEUEHEAD head;			// ����ڵ㣺����ʱ������
	unsigned long timestamp;		// ʱ����������ʱ��
	unsigned long size;				// ��С
	unsigned char *data;			// ����ָ��
};

typedef struct ISIMPACKET iSimPacket;

// ������·
struct ISIMTRANSFER
{
	struct IQUEUEHEAD head;			// ʱ����������
	unsigned long current;			// ��ǰʱ��
	unsigned long seed;				// �������
	long size;						// ������
	long limit;						// ������
	long rtt;						// ƽ������ʱ��(120, 200, ..)
	long lost;						// �����ʰٷֱ�(0-100)
	long amb;						// �ӳ�����ٷֱ�(0-100)
	int mode;						// ģʽ0(��ǰ�󵽴�)1(˳�򵽴�)
	long cnt_send;					// �����˶��ٸ���
	long cnt_drop;					// ��ʧ�˶��ٸ���
};

typedef struct ISIMTRANSFER iSimTransfer;

// ����˵�
struct ISIMPEER
{
	iSimTransfer *t1;				// ������·
	iSimTransfer *t2;				// ������·
};

typedef struct ISIMPEER iSimPeer;

// ����ģ����
struct ISIMNET
{
	iSimTransfer t1;				// ��·1
	iSimTransfer t2;				// ��·2
	iSimPeer p1;					// �˵�1 (t1, t2)
	iSimPeer p2;					// �˵�2 (t2, t1)
};

typedef struct ISIMNET iSimNet;


#ifdef __cplusplus
extern "C" {
#endif

/*====================================================================*/
/* INTERFACE DEFINITION                                               */
/*====================================================================*/

// ������·����ʼ��
void isim_transfer_init(iSimTransfer *trans, long rtt, long lost, long amb, 
		long limit, int mode);

// ������·������
void isim_transfer_destroy(iSimTransfer *trans);

// ������·������ʱ��
void isim_transfer_settime(iSimTransfer *trans, unsigned long time);

// ������·�������
long isim_transfer_random(iSimTransfer *trans, long range);

// ������·����������
long isim_transfer_send(iSimTransfer *trans, const void *data, long size);

// ������·����������
long isim_transfer_recv(iSimTransfer *trans, void *data, long maxsize);



// isim_init:
// ��ʼ������ģ����
// rtt   - ����ʱ��ƽ����
// lost  - �����ʰٷֱ� (0 - 100)
// amb   - ʱ������ٷֱ� (0 - 100)
// limit - ������������
// mode  - 0(�󷢰����ȵ�) 1(�󷢰���Ȼ�󵽴�)
// ����ʱ��  = ��ǰʱ�� + rtt * 0.5 + rtt * (amb * 0.01) * random(-0.5, 0.5)
// ��������  = rtt( 60), lost( 5), amb(30), limit(1000)
// ��������  = rtt(120), lost(10), amb(40), limit(1000)
// ������ͨ  = rtt(200), lost(10), amb(50), limit(1000)
// ��������  = rtt(800), lost(20), amb(60), limit(1000)
void isim_init(iSimNet *simnet, long rtt, long lost, long amb, long limit, int mode);

// ɾ������ģ����
void isim_destroy(iSimNet *simnet);

// ����ʱ��
void isim_settime(iSimNet *simnet, unsigned long current);


// ��������
long isim_send(iSimPeer *peer, const void *data, long size);

// ��������
long isim_recv(iSimPeer *peer, void *data, long maxsize);

// ȡ�ö˵㣺peerno = 0(�˵�1), 1(�˵�2)
iSimPeer *isim_peer(iSimNet *simnet, int peerno);

// �������������
void isim_seed(iSimNet *simnet, unsigned long seed1, unsigned long seed2);



#ifdef __cplusplus
}
#endif

#endif



