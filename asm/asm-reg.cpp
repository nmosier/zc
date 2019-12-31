#include "asm.hpp"

namespace zc::z80 {

   const ByteRegister r_a("a"), r_b("b"), r_c("c"), r_d("d"), r_e("e"), r_f("f"), r_h("h"),
      r_l("l"), r_ixh("ixh"), r_ixl("ixl"), r_iyh("iyh"), r_iyl("iyl");
   
   const MultibyteRegister r_af(MultibyteRegister::ByteRegs{&r_a, &r_f}, "af"),
      r_bc(MultibyteRegister::ByteRegs{&r_b, &r_c}, "bc"),
      r_de(MultibyteRegister::ByteRegs{&r_d, &r_e}, "de"),
      r_hl(MultibyteRegister::ByteRegs{&r_h, &r_l}, "hl"),
      r_ix(MultibyteRegister::ByteRegs{&r_ixh, &r_ixl}, "ix"),
      r_iy(MultibyteRegister::ByteRegs{&r_iyh, &r_iyl}, "iy"),
      r_sp(MultibyteRegister::ByteRegs{nullptr, nullptr}, "sp");
   

}
