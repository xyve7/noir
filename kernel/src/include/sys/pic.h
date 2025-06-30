#pragma once

// Disable the PIC
// This is needed so we can initialize the LAPIC
void pic_disable();
