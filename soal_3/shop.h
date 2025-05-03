#ifndef SHOP_H
#define SHOP_H

#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_INVENTORY 10

typedef struct {
    char name[32];
    int damage;
    int price;
    char passive[64];
} Weapon;


void show_weapon_shop(int socket_fd);
int buy_weapon(int id, int *gold, char **equipped_weapon, int *base_damage, char *response, size_t size);

#endif 