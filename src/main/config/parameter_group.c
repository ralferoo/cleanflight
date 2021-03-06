/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stddef.h>
#include <string.h>
#include <stdint.h>

#include "parameter_group.h"
#include "common/maths.h"

const pgRegistry_t* pgFind(pgn_t pgn)
{
    PG_FOREACH(reg) {
        if (pgN(reg) == pgn) {
            return reg;
        }
    }
    return NULL;
}

const pgRegistry_t* pgMatcher(pgMatcherFuncPtr matcher, const void *criteria)
{
    PG_FOREACH(candidate) {
        if (matcher(candidate, criteria)) {
            return candidate;
        }
    }
    return NULL;
}

static uint8_t *pgOffset(const pgRegistry_t* reg, uint8_t profileIndex)
{
    const uint16_t regSize = pgSize(reg);

    uint8_t *base = reg->address;
    if (!pgIsSystem(reg)) {
        base += (regSize * profileIndex);
    }
    return base;
}

static void pgResetInstance(const pgRegistry_t *reg, uint8_t profileIndex)
{
    const uint16_t regSize = pgSize(reg);

    uint8_t *base = pgOffset(reg, profileIndex);

    memset(base, 0, regSize);
    if (reg->resetFunc) {
        reg->resetFunc(base);
    }
}


void pgLoad(const pgRegistry_t* reg, const void *from, int size, uint8_t profileIndex)
{
    pgResetInstance(reg, profileIndex);

    const int take = MIN(size, pgSize(reg));
    memcpy(pgOffset(reg, profileIndex), from, take);
}

void pgResetAll(uint8_t profileCount)
{
    PG_FOREACH(reg) {
        if (pgIsSystem(reg)) {
            pgResetInstance(reg, 0);
        } else {
            // reset one instance for each profile
            for (uint8_t profileIndex = 0; profileIndex < profileCount; profileIndex++) {
                pgResetInstance(reg, profileIndex);
            }
        }
    }
}

void pgActivateProfile(uint8_t profileIndexToActivate)
{
    PG_FOREACH(reg) {
        if (!pgIsSystem(reg)) {
            uint8_t *ptr = pgOffset(reg, profileIndexToActivate);
            *(reg->ptr) = ptr;
        }
    }
}

