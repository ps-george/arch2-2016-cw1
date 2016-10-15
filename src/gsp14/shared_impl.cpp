/*
 * shared_impl.cpp
 *
 *  Created on: 15 Oct 2016
 *      Author: big-g
 */
#include <map>

const std::map<uint8_t, std::string> opcode_to_str{
	{0x00,"R"}, //! \todo
	{0x01,"BCND"}, //! \todo
	{0x02,"J"}, //! \todo
	{0x03,"JAL"}, //! \todo
	{0x04,"BEQ"}, //! \todo
	{0x05,"BNE"}, //! \todo
	{0x06,"BLEZ"}, //! \todo
	{0x07,"BGTZ"}, //! \todo
	{0x08,"ADDI"}, //! \todo
	{0x09,"ADDIU"}, //! \todo
	{0x0a,"SLTI"}, //! \todo
	{0x0b,"SLTIU"}, //! \todo
	{0x0c,"ANDI"},//! \todo
	{0x0d,"ORI"}, //! \todo
	{0x0e,"XORI"}, //! \todo
	{0x0f,"LUI"}, //! \todo
	{0x10,"ni"},
	{0x11,"ni"},
	{0x12,"ni"},
	{0x13,"ni"},
	{0x14,"nv"},
	{0x15,"nv"},
	{0x16,"nv"},
	{0x17,"nv"},
	{0x18,"nv"},
	{0x19,"nv"},
	{0x1a,"nv"},
	{0x1b,"nv"},
	{0x1c,"nv"},
	{0x1d,"nv"},
	{0x1e,"nv"},
	{0x1f,"nv"},
	{0x20,"LB"}, //! \todo
	{0x21,"LH"}, //! \todo
	{0x22,"LWL"}, //! \todo
	{0x23,"LW"}, //! \todo
	{0x24,"LBU"}, //! \todo
	{0x25,"LHU"}, //! \todo
	{0x26,"LWR"}, //! \todo
	{0x27,"nv"},
	{0x28,"SB"}, //! \todo
	{0x29,"SH"}, //! \todo
	{0x2a,"SWL"},
	{0x2b,"SW"}, //! \todo 26
	{0x2c,"nv"},
	{0x2d,"nv"},
	{0x2e,"SWR"}, //!
	{0x2f,"nv"},
	{0x30,"ni"},
	{0x31,"ni"},
	{0x32,"ni"},
	{0x33,"ni"},
	{0x34,"nv"},
	{0x35,"nv"},
	{0x36,"nv"},
	{0x37,"nv"},
	{0x38,"ni"},
	{0x39,"ni"},
	{0x3a,"ni"},
	{0x3b,"ni"},
	{0x3c,"nv"},
	{0x3d,"nv"},
	{0x3e,"nv"},
	{0x3f,"nv"}
};


