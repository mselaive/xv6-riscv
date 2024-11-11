#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// mprotect tarea 3

uint64
sys_mprotect(void)
{
  uint64 addr;
  int len;

  // Obtener los argumentos pasados a la función (dirección base y cantidad de páginas)
  argaddr(0, &addr);
  argint(1, &len);

  // Validar que la dirección esté alineada a páginas y que la longitud sea positiva
  if (addr % PGSIZE != 0 || len <= 0) {
    return -1;
  }

  // Recorrer la cantidad de páginas especificada por len
  for (int i = 0; i < len; i++) {

    // Localizar la entrada de tabla de páginas (PTE) para la dirección actual
    pte_t *pte = walk(myproc()->pagetable, addr + i * PGSIZE, 0);
    
    // Verificar que la entrada de la página exista y sea válida
    if (!pte || !(*pte & PTE_V)){
      return -1;
    }

    // Desactivar el permiso de escritura cambiando el bit correspondiente
    *pte &= ~PTE_W;
  }
    return 0;
}

// munprotect tarea 3

uint64 sys_munprotect(void)
{
  uint64 addr;
  int len;

  // Obtener dirección y longitud
  argaddr(0, &addr);
  argint(1, &len);

  // Comprobar si la dirección es múltiplo de PGSIZE y longitud válida
  if (len <= 0 || addr % PGSIZE != 0) {
    return -1;
  }

  // Iterar sobre las páginas afectadas
  for (int i = 0; i < len; i++) {
    pte_t *pte = walk(myproc()->pagetable, addr + i * PGSIZE, 0);

    // Verificar si la PTE es válida
    if (!pte || !(*pte & PTE_V)) {
      return -1;
    }

    // Habilitar la escritura (cambiar el bit W)
    *pte |= PTE_W;
  }
  return 0;
}

