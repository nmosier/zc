#include "asm.hpp"

namespace zc::z80 {

   const ByteRegister r_a("a"), r_b("b"), r_c("c"), r_d("d"), r_e("e"), r_f("f"), r_h("h"),
      r_l("l"), r_ixh("ixh"), r_ixl("ixl"), r_iyh("iyh"), r_iyl("iyl");
   
   const MultibyteRegister r_af(MultibyteRegister::ByteRegs{&r_a, &r_f}, "af");
   const MultibyteRegister r_bc(MultibyteRegister::ByteRegs{&r_b, &r_c}, "bc");
   const MultibyteRegister r_de(MultibyteRegister::ByteRegs{&r_d, &r_e}, "de");
   const MultibyteRegister r_hl(MultibyteRegister::ByteRegs{&r_h, &r_l}, "hl");
   const MultibyteRegister r_ix(MultibyteRegister::ByteRegs{&r_ixh, &r_ixl}, "ix");
   const MultibyteRegister r_iy(MultibyteRegister::ByteRegs{&r_iyh, &r_iyl}, "iy");

}
