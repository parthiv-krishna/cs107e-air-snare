.section .init
.globl _start
_start:
    cps     #0x1F
    mov sp, #0x4000000
    mov fp, #0
    bl _cstart
hang: b hang

.global EnableMMU
EnableMMU:
    mov     r1, #0
    # Invalidate d/i/unified caches (ARM ARM p. B6-21)
    mcr     p15, 0, r1, c7, c7, 0
    # Invalidate d/i/unified TLBs (ARM ARM p. B4-45)
    mcr     p15, 0, r1, c8, c7, 0
    # Tell the coprocessor about the table address (ARM ARM p. B4-41/B5-18)
    orr     r0, #1
    mcr     p15, 0, r0, c2, c0, 0
    mcr     p15, 0, r0, c2, c0, 1
    # Set domain control access to Manager (ARM ARM p. B4-10/B5-18)
    # Also http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0360f/CHDGIJFB.html
    mov     r1, #0xf
    mcr     p15, 0, r1, c3, c0, 0
    # Turn on MMU, with i/d caches (ARM ARM p. B3-12/B4-40/B5-18/B6-18)
    mrc     p15, 0, r1, c1, c0, 0
    orr     r1, r1, #0xd    // MMU & d-cache (B3-12)
    orr     r1, r1, #0x1f00 // i-cache & branch prediction (B3-12)
    mcr     p15, 0, r1, c1, c0, 0
    bx      lr
