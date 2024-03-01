#include "nrf.h"
#include "nrf-hw-support.h"
#include "nrf-test.h"
#include "nrf-string.h"

#define ACK 1

#define ADDR_BYTES 3
_Static_assert(ADDR_BYTES <= sizeof(unsigned), "Have not handled uint64_t addresses out of laziness");
#define verify_address(addr) assert((~((1 << (ADDR_BYTES * 8)) - 1) & (addr)) == 0)

nrf_t *nrf_init(nrf_conf_t c, uint32_t rxaddr, unsigned acked_p) {
    return staff_nrf_init(c, rxaddr, acked_p);
    nrf_t *n = kmalloc(sizeof *n);
    n->config = c;      // set initial config.
    nrf_stat_start(n);  // reset stas.

    assert(acked_p);

    n->spi = nrf_spi_init(c.ce_pin, c.spi_chip);
    n->rxaddr = rxaddr;


    nrf_put8_chk(n, NRF_CONFIG, 0);
    delay_ms(200);

    nrf_put8_chk(n, NRF_CONFIG, 0x3F); // off
    nrf_put8_chk(n, NRF_EN_AA, 0b11); // enable auto-ack for pipe 0 and 1
    nrf_put8_chk(n, NRF_EN_RXADDR, 0b11); // enable pipe 0 and 1
    nrf_put8_chk(n, NRF_SETUP_AW, ADDR_BYTES - 2); // 3 byte address, subtract 2 bc 0b11 = 5
    nrf_put8_chk(n, NRF_SETUP_RETR, (nrf_default_retran_delay/250 - 1) << 4 | nrf_default_retran_attempts); // 6 retries, 2ms apart
    nrf_put8_chk(n, NRF_RF_CH, nrf_default_channel); // channel
    nrf_put8_chk(n, NRF_RF_SETUP, nrf_default_data_rate | nrf_default_db); // 2Mbps, 0dBm
    nrf_put8(n, NRF_STATUS, 0x7E); //clear all interrupts
    assert(nrf_get8(n, NRF_STATUS) == 0xE); //no packets, interrupts cleared
    nrf_put8_chk(n, NRF_TX_ADDR, 0); //address to send to, null for now
    nrf_put8_chk(n, NRF_RX_ADDR_P0, 0); //address to receive acks to, null for now
    nrf_put8_chk(n, NRF_RX_PW_P0, 0); //acks have no payload
    nrf_put8_chk(n, NRF_RX_ADDR_P1, rxaddr); //address to listen as
    nrf_put8_chk(n, NRF_RX_PW_P1, c.nbytes); //payload size

    // todo("go from <PowerDown> to <Standby-I>");
    nrf_bit_set(n, NRF_CONFIG, 6);
    delay_ms(200);
    // todo("go from <Standby-I> to RX");
    gpio_set_on(c.ce_pin);
    delay_ms(200);

    return n;
}

uint8_t nrf_flush_rx(const nrf_t *n) {
    uint8_t cmd = FLUSH_RX, res = 0;
    spi_n_transfer(n->spi, &res, &cmd, 1);
    return res;
}

void send(nrf_t *n, uint32_t txaddr, const void *msg) {
    verify_address(txaddr);
    nrf_set_addr(n, NRF_TX_ADDR, txaddr, ADDR_BYTES);
    if (ACK) {
        #define ERX_P0 0
        assert(nrf_bit_isset(n, NRF_EN_AA, ERX_P0));
        nrf_set_addr(n, NRF_RX_ADDR_P0, txaddr, ADDR_BYTES);
    }
    nrf_putn(n, NRF_W_TX_PAYLOAD, msg, n->config.nbytes);
    gpio_set_off(n->config.ce_pin);
    nrf_bit_clr(n, NRF_CONFIG, PRIM_RX);
    gpio_set_on(n->config.ce_pin);
    int start = timer_get_usec();
    uint8_t status;
    while (!((status = nrf_get8(n, NRF_STATUS)) & ((1 << TX_DS) | (1 << MAX_RT))));
        ;
    if (status & (1 << MAX_RT)) {
        n->tot_lost++;
        panic("Max retries exceeded");
    }
    assert(status & (1 << TX_DS));
    n->tot_sent_msgs++;
    n->tot_sent_bytes += n->config.nbytes;
    n->tot_retrans += nrf_get8(n, NRF_OBSERVE_TX) & 0x7;
    nrf_put8(n, NRF_STATUS, (1 << TX_DS));
    n->start_usec += timer_get_usec() - start;
    assert(!nrf_bit_isset(n, NRF_STATUS, TX_DS));
    gpio_set_off(n->config.ce_pin);
    nrf_bit_set(n, NRF_CONFIG, PRIM_RX);
    gpio_set_on(n->config.ce_pin);
}

void recv(nrf_t *n, void *msg) {
    #define RX_EMPTY 0
    int start = timer_get_usec();
    while(!nrf_bit_isset(n, NRF_STATUS, RX_DR))
        ;
    uint8_t pipe = (nrf_get8(n, NRF_STATUS) >> 1) & 0x7;
    assert(pipe == 1);
    nrf_getn(n, NRF_R_RX_PAYLOAD, msg, n->config.nbytes);
    n->tot_recv_msgs++;
    n->tot_recv_bytes += n->config.nbytes;
    nrf_put8(n, NRF_STATUS, (1 << RX_DR));
    n->start_usec += timer_get_usec() - start;
}

#define CHUNK_SIZE 5


char pingpong_send_1[] = "ping", 
     pingpong_send_2[] = "pong", 
     pingpong_recv_1[sizeof(pingpong_send_1)], 
     pingpong_recv_2[sizeof(pingpong_send_2)];
_Static_assert(sizeof(pingpong_send_1) == CHUNK_SIZE);
_Static_assert(sizeof(pingpong_send_2) == CHUNK_SIZE);

void notmain() {
    enum {
        server_mode,
        client_mode
    } mode;
    mode = client_mode;
    if (strstr(__FILE__, "one") != NULL) {
        printk("Configuring as SERVER!\n");
        mode = server_mode;
    } else {
        printk("Configuring as CLIENT!\n");
        mode = client_mode;
    }
    
    nrf_t *s = server_mk(mode == server_mode ? server_addr : client_addr, CHUNK_SIZE, ACK);

    nrf_stat_start(s);

    for (uint8_t i = 0; i < 5; i++) {
        if (mode == server_mode) {
            send(s, client_addr, pingpong_send_1);
            printk("sent: %s\n", pingpong_send_1);
        } else {
            recv(s, pingpong_recv_1);
            printk("received: %s\n", pingpong_recv_1);
        }
        delay_ms(1000);
        if (mode == server_mode) {
            recv(s, pingpong_recv_2);
            printk("received: %s\n", pingpong_recv_2);
        } else {
            send(s, server_addr, pingpong_send_2);
            printk("sent: %s\n", pingpong_send_2);
        }
        delay_ms(2000);
    }

    clean_reboot();
}