/*
 * Copyright (C) 2010 Ciriaco Garcia de Celis
 * Copyright (C) 2012 Vital Kolas
 *
 * This program is  free software:  you can redistribute it and/or
 * modify it under the terms of the  GNU General Public License as
 * published by the Free Software Foundation,  either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY  or  FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License (GPL) for more details.
 *
 * You should have received a copy of the GNU  GPL along with this
 * program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <climits>
#include <errno.h>

#include "list.h"
#include "netif.h"

#define STATE_FILE_BASE "/dev/shm/netmon"

/******************************************************************************/
/* Print last error and exit app                                              */
/******************************************************************************/
int exitapp(int exitcode)
{
    char errmsg[64];
    sprintf(errmsg, "ERROR code %d", exitcode);
    if (write(1, errmsg, strlen(errmsg)) == -1)
    {
        exitapp(errno);
    }
    return exitcode;
}

/******************************************************************************/
/* Main entry point                                                           */
/******************************************************************************/
int main(int argc, char** argv)
{
    if (argc < 2)
    {
         printf("usage: %s <network_interface>\n", argv[0]);
         //return 1;
    }

    char buffer[4096], cad[256], *ni, *nf;

    // read network information
    int fd = open("/proc/net/dev", O_RDONLY);
    if (fd < 0)
    {
        return exitapp(2);
    }

    int bytes = read(fd, buffer, sizeof(buffer)-1);
    close(fd);

    if (bytes < 0)
    {
        return exitapp(3);
    }
    buffer[bytes] = 0;

    list_t list = iflist(buffer);

    if (argc < 2)
    {
        node_t iter = list->head;
        while (iter)
        {
            interface_t pif = (interface_t) iter->data;
            printf("%s %ld/%ld\n", pif->name, pif->recv, pif->sent);
            iter = iter->next;
        }

        list_destroy(list);

        return 1;
    }

    timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    long nanoseconds = tp.tv_sec * 1000000000LL + tp.tv_nsec;

    long recv_bytes=LLONG_MAX, sent_bytes=LLONG_MAX;
    bool networkAvailable = false;

    // search for the proper network interface
    strcpy(cad, argv[1]);
    strcat(cad, ":");

    interface_t pif = 0;
    node_t iter = list->head;
    while (iter)
    {
        interface_t ift = (interface_t) iter->data;
        if(strcmp(cad, ift->name) == 0)
        {
            pif = ift;
            break;
        }
        iter = iter->next;
    }

    if(pif != 0)
    {
        networkAvailable = true;
        recv_bytes = pif->recv;
        sent_bytes = pif->sent;
    }

    // read the received/sent bytes, date and CPU usage stored by a previous execution
    sprintf(cad, "%s.%s.%d", STATE_FILE_BASE, argv[1], getuid());
    fd = open(cad, O_RDWR | O_CREAT, 0664);
    if (fd < 0) return exitapp(6);
    bytes = read(fd, buffer, sizeof(buffer)-1);
    if (bytes < 0)
    {
        close(fd);
        return exitapp(7);
    }
    long prev_recv_bytes, prev_sent_bytes, prev_nanoseconds = -1;
    if (bytes > 0)
    {
        prev_recv_bytes = atoll(buffer);
        prev_sent_bytes = atoll(buffer+20);
        prev_nanoseconds = atoll(buffer+40);
    }

    // store in the file the current values for later use
    sprintf(buffer, "%019ld\n%019ld\n%019ld\n", recv_bytes, sent_bytes, nanoseconds);
    lseek(fd, 0, SEEK_SET);
    if (write(fd, buffer, 140) == -1)
    {
        exitapp(errno);
    }
    close(fd);

    // generate the result

    strcpy(buffer, "<txt>");

    bool hasNet = networkAvailable && (prev_nanoseconds >= 0) 
        && (recv_bytes >= prev_recv_bytes) && (sent_bytes >= prev_sent_bytes);

    if (!networkAvailable)
    {
        sprintf(cad, " %s is down", argv[1]);
        strcat(buffer, cad);
    }
    else if (!hasNet)
    {
        strcat(buffer, " ? kbps\n ? kbps");
    }
    else
    {
        long elapsed = nanoseconds - prev_nanoseconds;
        if (elapsed < 1) elapsed = 1;
        double seconds = elapsed / 1000000000.0;

        long sent = sent_bytes - prev_sent_bytes;
        long received = recv_bytes - prev_recv_bytes;
        long inbps = (long) (received / seconds);
        long outbps = (long) (sent / seconds);

        if (inbps < 1000000)
            sprintf(cad, "%4ld kB/", inbps/1000);
        else
            sprintf(cad, "%4.2f MB/", inbps/1000000.0);
        strcat(buffer, cad);

        if (recv_bytes < 1000000)
            sprintf(cad, "%6ld kB\n", recv_bytes/1000);
        else if (recv_bytes < 1000000000)
            sprintf(cad, "%6.2f MB\n", recv_bytes/1000000.0);
        else
            sprintf(cad, "%6.2f GB\n", recv_bytes/1000000000.0);
        strcat(buffer, cad);

        if (outbps < 1000000)
            sprintf(cad, "%4ld kB/", outbps/1000);
        else
            sprintf(cad, "%4.2f MB/", outbps/1000000.0);
        strcat(buffer, cad);

        if (sent_bytes < 1000000)
            sprintf(cad, "%6ld kB\n", sent_bytes/1000);
        else if (sent_bytes < 1000000000)
            sprintf(cad, "%6.2f MB\n", sent_bytes/1000000.0);
        else
            sprintf(cad, "%6.2f GB\n", sent_bytes/1000000000.0);
        strcat(buffer, cad);
    }

    strcat(buffer, "</txt><tool>");

    iter = list->head;
    while (iter)
    {
        interface_t ift = (interface_t) iter->data;
            
        sprintf(cad, " %s\n    %.2f MB received \n    %.2f MB sent ",
                    ift->name, ift->recv/1000000.0, ift->sent/1000000.0);
        if (iter->next)
        {
            strcat(cad, "\n");
        }
        strcat(buffer, cad);

        iter = iter->next;
    }

    strcat(buffer, "</tool>");

    if (write(1, buffer, strlen(buffer)) == -1)
    {
        exitapp(errno);
    }

    list_destroy(list);

    return 0;
}

