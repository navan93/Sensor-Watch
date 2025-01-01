/*
 * MIT License
 *
 * Copyright (c) 2023 Edward Shin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "shell_cmd_list.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filesystem.h"
#include "watch.h"
#include "hpl_dma.h"

#define NO_RADIO_RX_PIN     A4

static int help_cmd(int argc, char *argv[]);
static int adc_cmd(int argc, char *argv[]);
static int flash_cmd(int argc, char *argv[]);
static int stress_cmd(int argc, char *argv[]);

shell_command_t g_shell_commands[] = {
    {
        .name = "?",
        .help = "print command list",
        .min_args = 0,
        .max_args = 0,
        .cb = help_cmd,
    },
    {
        .name = "help",
        .help = "print command list",
        .min_args = 0,
        .max_args = 0,
        .cb = help_cmd,
    },
    {
        .name = "flash",
        .help = "reboot to UF2 bootloader",
        .min_args = 0,
        .max_args = 0,
        .cb = flash_cmd,
    },
    {
        .name = "ls",
        .help = "usage: ls [PATH]",
        .min_args = 0,
        .max_args = 1,
        .cb = filesystem_cmd_ls,
    },
    {
        .name = "cat",
        .help = "usage: cat <PATH>",
        .min_args = 1,
        .max_args = 1,
        .cb = filesystem_cmd_cat,
    },
    {
        .name = "df",
        .help = "print filesystem free space",
        .min_args = 0,
        .max_args = 0,
        .cb = filesystem_cmd_df,
    },
    {
        .name = "rm",
        .help = "usage: rm [PATH]",
        .min_args = 1,
        .max_args = 1,
        .cb = filesystem_cmd_rm,
    },
    {
        .name = "format",
        .help = "usage: format YES",
        .min_args = 1,
        .max_args = 1,
        .cb = filesystem_cmd_format,
    },
    {
        .name = "echo",
        .help = "usage: echo TEXT {>,>>} FILE",
        .min_args = 3,
        .max_args = 3,
        .cb = filesystem_cmd_echo,
    },
    {
        .name = "stress",
        .help = "test CDC write; usage: stress [LEN] [DELAY_MS]",
        .min_args = 0,
        .max_args = 2,
        .cb = stress_cmd,
    },
    {
        .name = "adc",
        .help = "capture ADC",
        .min_args = 0,
        .max_args = 0,
        .cb = adc_cmd,
    },
};

const size_t g_num_shell_commands = sizeof(g_shell_commands) / sizeof(shell_command_t);

static int help_cmd(int argc, char *argv[]) {
    (void) argc;
    (void) argv;

    printf("Command List:\r\n");
    for (size_t i = 0; i < g_num_shell_commands; i++) {
        printf(" %s\t%s\r\n",
                g_shell_commands[i].name,
                (g_shell_commands[i].help) ? g_shell_commands[i].help : ""
        );
    }

    return 0;
}

static int flash_cmd(int argc, char *argv[]) {
    (void) argc;
    (void) argv;

    watch_reset_to_bootloader();
    return 0;
}

#define STRESS_CMD_MAX_LEN  (512)
static int stress_cmd(int argc, char *argv[]) {
    char test_str[STRESS_CMD_MAX_LEN+1] = {0};

    int max_len = 512;
    int delay = 0;

    if (argc >= 2) {
        if ((max_len = atoi(argv[1])) == 0) {
            return -1;
        }
        if (max_len > 512) {
            return -1;
        }
    }

    if (argc >= 3) {
        delay = atoi(argv[2]);
    }

    for (int i = 0; i < max_len; i++) {
        snprintf(&test_str[i], 2, "%u", (i+1)%10);
        printf("%u:\t%s\r\n", (i+1), test_str);
        if (delay > 0) {
            delay_ms(delay);
        }
    }

    return 0;
}

/* Trigger action = DMA_TRIGGER_ACTON_BEAT
   peripheral_trigger = ADC_DMAC_ID_RESRDY
   beat_size = DMA_BEAT_SIZE_BYTE
   block_transfer_count = 1000
   block_transfer_count = DMA_BLOCK_ACTION_INT
   src_increment_enable = false
   source_address = (uint32_t)(&ADC->RESULT.reg)

*/
static uint8_t m_adc_buffer[150];

static void dma_xfer_done(struct _dma_resource *resource)
{
    (void)resource;
    printf("DMA Transfer completed\r\n");
    adc_stop();
    // printf("ADC value on A4: %d\r\n", value);
    printf("ADC Buffer:");
    for (uint16_t i = 0; i < sizeof(m_adc_buffer); i++) {
        printf("%d ", m_adc_buffer[i]);
    }
}
static void dma_init(void)
{
    struct _dma_resource *ch0_resource;
    m_adc_buffer[0] = 10;

    _dma_set_source_address(0, (void*)&ADC->RESULT.reg);
    _dma_srcinc_enable(0, false);
    _dma_set_destination_address(0, (void*)m_adc_buffer);
    _dma_dstinc_enable(0, true);
    _dma_set_data_amount(0, 100);
    _dma_set_irq_state(0, DMA_TRANSFER_COMPLETE_CB, true);
    _dma_get_channel_resource(&ch0_resource, 0);
    ch0_resource->dma_cb.transfer_done = dma_xfer_done;
    _dma_enable_transaction(0, false);
}

static int adc_cmd(int argc, char *argv[]) {
    (void) argc;
    (void) argv;

    printf("Capture ADC samples on A4\r\n");
    memset(m_adc_buffer, 0, sizeof(m_adc_buffer));

    // Enable the ADC peripheral, which we'll use to read the thermistor value.
    watch_enable_adc_dma();
    watch_set_analog_sampling_length(1);
    // Enable analog circuitry on the sense pin, which is tied to the thermistor resistor divider.
    watch_enable_analog_input(A4);

    // Configure DMA
    dma_init();

    // get the sense pin level
    // uint16_t value = watch_get_analog_pin_level(NO_RADIO_RX_PIN);
    adc_start(NO_RADIO_RX_PIN);

    // printf("ADC value on A4: %d\r\n", value);
    // printf("ADC Buffer:");
    // for (uint16_t i = 0; i < sizeof(m_adc_buffer); i++) {
    //     printf("%d ", m_adc_buffer[i]);
    // }
    return 0;
}