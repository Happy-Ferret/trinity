#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <linux/atm.h>
#include <linux/dn.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/llc.h>
#include <linux/if_packet.h>
#include <neteconet/ec.h>
#include <linux/irda.h>
#include <linux/can.h>
#include <linux/tipc.h>
#include <linux/phonet.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include "sanitise.h"
#include "compat.h"
#include "net.h"
#include "maps.h"
#include "config.h"
#include "params.h"	// do_specific_proto

static void gen_decnet(unsigned long *addr, unsigned long *addrlen)
{
	struct sockaddr_dn *dn;
	unsigned int i;

	dn = malloc(sizeof(struct sockaddr_dn));
	if (dn == NULL)
		return;

	dn->sdn_family = PF_DECnet;
	dn->sdn_flags = rand();
	dn->sdn_objnum = rand();
	dn->sdn_objnamel = rand() % 16;
	for (i = 0; i < dn->sdn_objnamel; i++)
		dn->sdn_objname[i] = rand();
	dn->sdn_add.a_len = rand() % 2;
	dn->sdn_add.a_addr[0] = rand();
	dn->sdn_add.a_addr[1] = rand();
	*addr = (unsigned long) dn;
	*addrlen = sizeof(struct sockaddr_dn);
}

static void gen_llc(unsigned long *addr, unsigned long *addrlen)
{
	struct sockaddr_llc *llc;
	unsigned int i;

	llc = malloc(sizeof(struct sockaddr_llc));
	if (llc == NULL)
		return;
	llc->sllc_family = AF_LLC;
	llc->sllc_arphrd = ARPHRD_ETHER;
	llc->sllc_test = rand();
	llc->sllc_xid = rand();
	llc->sllc_ua = rand();
	llc->sllc_sap = rand();
	for (i = 0; i < IFHWADDRLEN; i++)
		llc->sllc_mac[i] = rand();
	*addr = (unsigned long) llc;
	*addrlen = sizeof(struct sockaddr_llc);
}

static void gen_netlink(unsigned long *addr, unsigned long *addrlen)
{
	struct sockaddr_nl *nl;

	nl = malloc(sizeof(struct sockaddr_nl));
	if (nl == NULL)
		return;

	nl->nl_family = PF_NETLINK;
	nl->nl_pid = rand();
	nl->nl_groups = rand();
	*addr = (unsigned long) nl;
	*addrlen = sizeof(struct sockaddr_nl);
}

static void gen_packet(unsigned long *addr, unsigned long *addrlen)
{
	struct sockaddr_pkt *pkt;
	unsigned int i;

	//TODO: See also sockaddr_ll
	pkt = malloc(sizeof(struct sockaddr_pkt));
	if (pkt == NULL)
		return;

	pkt->spkt_family = PF_PACKET;
	for (i = 0; i < 14; i++)
		pkt->spkt_device[i] = rand();
	*addr = (unsigned long) pkt;
	*addrlen = sizeof(struct sockaddr_pkt);
}

static void gen_econet(unsigned long *addr, unsigned long *addrlen)
{
	struct sockaddr_ec *ec;

	ec = malloc(sizeof(struct sockaddr_ec));
	if (ec == NULL)
		return;

	ec->sec_family = PF_ECONET;
	ec->port = rand();
	ec->cb = rand();
	ec->type = rand();
	ec->addr.station = rand();
	ec->addr.net = rand();
	ec->cookie = rand();
	*addr = (unsigned long) ec;
	*addrlen = sizeof(struct sockaddr_ec);
}

static void gen_atmsvc(unsigned long *addr, unsigned long *addrlen)
{
	struct sockaddr_atmsvc *atmsvc;
	unsigned int i;

	atmsvc = malloc(sizeof(struct sockaddr_atmsvc));
	if (atmsvc == NULL)
		return;

	atmsvc->sas_family = PF_ATMSVC;
	for (i = 0; i < ATM_ESA_LEN; i++)
		atmsvc->sas_addr.prv[i] = rand();
	for (i = 0; i < ATM_E164_LEN; i++)
		atmsvc->sas_addr.pub[i] = rand();
	atmsvc->sas_addr.lij_type = rand();
	atmsvc->sas_addr.lij_id = rand();
	*addr = (unsigned long) atmsvc;
	*addrlen = sizeof(struct sockaddr_atmsvc);
}

static void gen_irda(unsigned long *addr, unsigned long *addrlen)
{
	struct sockaddr_irda *irda;
	unsigned int i;

	irda = malloc(sizeof(struct sockaddr_irda));
	if (irda == NULL)
		return;

	irda->sir_family = PF_IRDA;
	irda->sir_lsap_sel = rand();
	irda->sir_addr = rand();
	for (i = 0; i < 25; i++)
		irda->sir_name[i] = rand();
	*addr = (unsigned long) irda;
	*addrlen = sizeof(struct sockaddr_irda);
}

static void gen_can(unsigned long *addr, unsigned long *addrlen)
{
	struct sockaddr_can *can;

	can = malloc(sizeof(struct sockaddr_can));
	if (can == NULL)
		return;
	can->can_family = AF_CAN;
	can->can_ifindex = rand();
	can->can_addr.tp.rx_id = rand();
	can->can_addr.tp.tx_id = rand();
	*addr = (unsigned long) can;
	*addrlen = sizeof(struct sockaddr_can);
}

static void gen_tipc(unsigned long *addr, unsigned long *addrlen)
{
	struct sockaddr_tipc *tipc;

	tipc = malloc(sizeof(struct sockaddr_tipc));
	if (tipc == NULL)
		return;
	tipc->family = AF_TIPC;
	tipc->addrtype = rand();
	tipc->scope = rand();
	tipc->addr.id.ref = rand();
	tipc->addr.id.node = rand();
	tipc->addr.nameseq.type = rand();
	tipc->addr.nameseq.lower = rand();
	tipc->addr.nameseq.upper = rand();
	tipc->addr.name.name.type = rand();
	tipc->addr.name.name.instance = rand();
	tipc->addr.name.domain = rand();
	*addr = (unsigned long) tipc;
	*addrlen = sizeof(struct sockaddr_tipc);
}

static void gen_phonet(unsigned long *addr, unsigned long *addrlen)
{
	struct sockaddr_pn *pn;

	pn = malloc(sizeof(struct sockaddr_pn));
	if (pn == NULL)
		return;

	pn->spn_family = PF_PHONET;
	pn->spn_obj = rand();
	pn->spn_dev = rand();
	pn->spn_resource = rand();
	*addr = (unsigned long) pn;
	*addrlen = sizeof(struct sockaddr_pn);
}

void generate_sockaddr(unsigned long *addr, unsigned long *addrlen, int pf)
{
	/* If we want sockets of a specific type, we'll want sockaddrs that match. */
	if (do_specific_proto == TRUE)
		pf = specific_proto;

	/* If we got no hint passed down, pick a random proto. */
	if (pf == -1)
		pf = rand() % TRINITY_PF_MAX;

	switch (pf) {

	case PF_UNSPEC:
		//TODO
		break;

	case PF_UNIX:
		gen_unixsock(addr, addrlen);
		break;

	case PF_INET:
		gen_ipv4(addr, addrlen);
		break;

	case PF_AX25:
		gen_ax25(addr, addrlen);
		break;

	case PF_IPX:
		gen_ipx(addr, addrlen);
		break;

	case PF_APPLETALK:
		gen_appletalk(addr, addrlen);
		break;

	case PF_NETROM:
		//TODO
		break;

	case PF_BRIDGE:
		//TODO
		break;

	case PF_ATMPVC:
		gen_atmpvc(addr, addrlen);
		break;

	case PF_X25:
		gen_x25(addr, addrlen);
		break;

	case PF_INET6:
		gen_ipv6(addr, addrlen);
		break;

	case PF_ROSE:
		gen_rose(addr, addrlen);
		break;

	case PF_DECnet:
		gen_decnet(addr, addrlen);
		break;

	case PF_NETBEUI:
		gen_llc(addr, addrlen);
		break;

	case PF_SECURITY:
		//TODO
		break;

	case PF_KEY:
		break;

	case PF_NETLINK:
		gen_netlink(addr, addrlen);
		break;

	case PF_PACKET:
		gen_packet(addr, addrlen);
		break;

	case PF_ASH:
		//TODO
		break;

	case PF_ECONET:
		gen_econet(addr, addrlen);
		break;

	case PF_ATMSVC:
		gen_atmsvc(addr, addrlen);
		break;

	case PF_RDS:
		//TODO
		break;

	case PF_SNA:
		//TODO
		break;

	case PF_IRDA:
		gen_irda(addr, addrlen);
		break;

	case PF_PPPOX:
		gen_pppox(addr, addrlen);
		break;

	case PF_WANPIPE:
		//TODO
		break;

	case PF_LLC:
		gen_llc(addr, addrlen);
		break;

	case PF_CAN:
		gen_can(addr, addrlen);
		break;

	case PF_TIPC:
		gen_tipc(addr, addrlen);
		break;

	case PF_BLUETOOTH:
		//TODO
		break;

	case PF_IUCV:
		//TODO
		break;

	case PF_RXRPC:
		//TODO
		break;

	case PF_ISDN:
		//TODO
		break;

	case PF_PHONET:
		gen_phonet(addr, addrlen);
		break;

	case PF_IEEE802154:
		//TODO
		break;

#ifdef USE_CAIF
	case PF_CAIF:
		gen_caif(addr, addrlen);
		break;
#endif

#ifdef USE_IF_ALG
	case PF_ALG:
		gen_alg(addr, addrlen);
		break;
#endif

	case PF_NFC:
		gen_nfc(addr, addrlen);
		break;

	case PF_VSOCK:
		//TODO
		break;

	default:
		break;
	}
}
