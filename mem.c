// The MIT License (MIT)
//
// Copyright (c) 2015 Stefan Arentz - http://github.com/st3fan/ewm
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#include "mem.h"

// The following two are our memory primitives that properly set go
// through the handler functions for all registered memory. They will
// take more time but do the right thing.

uint8_t mem_get_byte(struct cpu_t *cpu, uint16_t addr) {
  struct mem_t *mem = cpu->mem;
  while (mem != NULL) {
    if (addr >= mem->start && addr <= mem->end) {
      if (mem->read_handler) {
        return ((mem_read_handler_t) mem->read_handler)((struct cpu_t*) cpu, mem, addr);
      } else {
        if (cpu->strict) {
          // TODO: Signal an error about reading to write-only region (does that even exist?)
        }
        return 0;
      }
    }
    mem = mem->next;
  }

  if (cpu->strict) {
     // TODO: Signal an error about reading non-existent memory
  }

  return 0; // TODO What should the default be if we read from non-existent memory?
}

void mem_set_byte(struct cpu_t *cpu, uint16_t addr, uint8_t v) {
  struct mem_t *mem = cpu->mem;
  while (mem != NULL) {
    if (addr >= mem->start && addr <= mem->end) {
      if (mem->write_handler) {
        ((mem_write_handler_t) mem->write_handler)((struct cpu_t*) cpu, mem, addr, v);
      } else {
        if (cpu->strict) {
          // TODO: Signal an error about writing to read-only region
        }
      }
      return;
    }
    mem = mem->next;
  }

  if (cpu->strict) {
     // TODO: Signal an error about writing non-existent memory
  }
}

// Getters

uint8_t mem_get_byte_abs(struct cpu_t *cpu, uint16_t addr) {
  return mem_get_byte(cpu, addr);
}

uint8_t mem_get_byte_absx(struct cpu_t *cpu, uint16_t addr) {
  return mem_get_byte(cpu, addr + cpu->state.x); /* TODO: Carry? */
}

uint8_t mem_get_byte_absy(struct cpu_t *cpu, uint16_t addr) {
  return mem_get_byte(cpu, addr + cpu->state.y); /* TODO: Carry? */
}

uint8_t mem_get_byte_zpg(struct cpu_t *cpu, uint8_t addr) {
  return mem_get_byte(cpu, addr);
}

uint8_t mem_get_byte_zpgx(struct cpu_t *cpu, uint8_t addr) {
  return mem_get_byte(cpu, ((uint16_t) addr + cpu->state.x) & 0x00ff);
}

uint8_t mem_get_byte_zpgy(struct cpu_t *cpu, uint8_t addr) {
  return mem_get_byte(cpu, ((uint16_t) addr + cpu->state.y) & 0x00ff);
}

uint8_t mem_get_byte_indx(struct cpu_t *cpu, uint8_t addr) {
  return mem_get_byte(cpu, mem_get_word(cpu, (uint8_t)(addr + cpu->state.x)));
}

uint8_t mem_get_byte_indy(struct cpu_t *cpu, uint8_t addr) {
  return mem_get_byte(cpu, mem_get_word(cpu, addr) + cpu->state.y);
}

uint16_t mem_get_word(struct cpu_t *cpu, uint16_t addr) {
  // TODO Did I do this right?
  return ((uint16_t) mem_get_byte(cpu, addr+1) << 8) | (uint16_t) mem_get_byte(cpu, addr);
}

// Setters

void mem_set_byte_zpg(struct cpu_t *cpu, uint8_t addr, uint8_t v) {
  mem_set_byte(cpu, addr, v);
}

void mem_set_byte_zpgx(struct cpu_t *cpu, uint8_t addr, uint8_t v) {
  mem_set_byte(cpu, ((uint16_t) addr + cpu->state.x) & 0x00ff, v);
}

void mem_set_byte_zpgy(struct cpu_t *cpu, uint8_t addr, uint8_t v) {
  mem_set_byte(cpu, ((uint16_t) addr + cpu->state.y) & 0x00ff, v);
}

void mem_set_byte_abs(struct cpu_t *cpu, uint16_t addr, uint8_t v) {
  mem_set_byte(cpu, addr, v);
}

void mem_set_byte_absx(struct cpu_t *cpu, uint16_t addr, uint8_t v) {
  mem_set_byte(cpu, addr+cpu->state.x, v);
}

void mem_set_byte_absy(struct cpu_t *cpu, uint16_t addr, uint8_t v) {
  mem_set_byte(cpu, addr+cpu->state.y, v);
}

void mem_set_byte_indx(struct cpu_t *cpu, uint8_t addr, uint8_t v) {
  //uint8_t a = ;
  mem_set_byte(cpu, mem_get_word(cpu, (uint8_t)(addr + cpu->state.x)), v);
}

void mem_set_byte_indy(struct cpu_t *cpu, uint8_t addr, uint8_t v) {
  mem_set_byte(cpu, mem_get_word(cpu, addr)+cpu->state.y, v);
}

void mem_set_word(struct cpu_t *cpu, uint16_t addr, uint16_t v) {
  mem_set_byte(cpu, addr+0, (uint8_t) v); // TODO Did I do this right?
  mem_set_byte(cpu, addr+1, (uint8_t) (v >> 8));
}

/* MOD */

void mem_mod_byte_zpg(struct cpu_t *cpu, uint8_t addr, mem_mod_t op) {
  mem_set_byte_zpg(cpu, addr, op(cpu, mem_get_byte_zpg(cpu, addr)));
}

void mem_mod_byte_zpgx(struct cpu_t *cpu, uint8_t addr, mem_mod_t op) {
  mem_set_byte_zpgx(cpu, addr, op(cpu, mem_get_byte_zpgx(cpu, addr)));
}

void mem_mod_byte_zpgy(struct cpu_t *cpu, uint8_t addr, mem_mod_t op) {
  mem_set_byte_zpgy(cpu, addr, op(cpu, mem_get_byte_zpgy(cpu, addr)));
}

void mem_mod_byte_abs(struct cpu_t *cpu, uint16_t addr, mem_mod_t op) {
  mem_set_byte_abs(cpu, addr, op(cpu, mem_get_byte_abs(cpu, addr)));
}

void mem_mod_byte_absx(struct cpu_t *cpu, uint16_t addr, mem_mod_t op) {
  mem_set_byte_absx(cpu, addr, op(cpu, mem_get_byte_absx(cpu, addr)));
}

void mem_mod_byte_absy(struct cpu_t *cpu, uint16_t addr, mem_mod_t op) {
  mem_set_byte_absy(cpu, addr, op(cpu, mem_get_byte_absy(cpu, addr)));
}

void mem_mod_byte_indx(struct cpu_t *cpu, uint8_t addr, mem_mod_t op) {
  mem_set_byte_indx(cpu, addr, op(cpu, mem_get_byte_indx(cpu, addr)));
}

void mem_mod_byte_indy(struct cpu_t *cpu, uint8_t addr, mem_mod_t op) {
  mem_set_byte_indy(cpu, addr, op(cpu, mem_get_byte_indy(cpu, addr)));
}

// The following get and set memory directly. There are no checks, so
// make sure you are doing the right thing. Mainly used for managing
// the stack, reading instructions, reading vectors and tracing code.

uint8_t _mem_get_byte_direct(struct cpu_t *cpu, uint16_t addr) {
  assert(addr <= 0x200);
  return cpu->memory[addr];
}

uint16_t _mem_get_word_direct(struct cpu_t *cpu, uint16_t addr) {
  assert(addr <= 0x200);
  return *((uint16_t*) &cpu->memory[addr]);
}

void _mem_set_byte_direct(struct cpu_t *cpu, uint16_t addr, uint8_t v) {
  assert(addr <= 0x200);
  cpu->memory[addr] = v;
}

void _mem_set_word_direct(struct cpu_t *cpu, uint16_t addr, uint16_t v) {
  assert(addr <= 0x200);
  *((uint16_t*) &cpu->memory[addr]) = v;
}
