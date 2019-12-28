#include "asm.hpp"

namespace zc::z80 {

   const Register<Size::BYTE> r_a("a"), r_b("b"), r_c("c"), r_d("d"), r_e("e"), r_f("f"), r_h("h"),
      r_l("l"), r_ixh("ixh"), r_ixl("ixl"), r_iyh("iyh"), r_iyl("iyl");
   
   const Register<Size::LONG> r_af(Register<Size::LONG>::ByteRegs{&r_a, &r_f}, "af"),
      r_bc(Register<Size::LONG>::ByteRegs{&r_b, &r_c}, "bc"),
      r_de(Register<Size::LONG>::ByteRegs{&r_d, &r_e}, "de"),
      r_hl(Register<Size::LONG>::ByteRegs{&r_h, &r_l}, "hl"),
      r_ix(Register<Size::LONG>::ByteRegs{&r_ixh, &r_ixl}, "ix"),
      r_iy(Register<Size::LONG>::ByteRegs{&r_iyh, &r_iyl}, "iy");

}
