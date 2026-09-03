#include <core/engine/chain.hpp>

namespace cg::engine
{
	ChainNode::ChainNode() noexcept
		: id{static_cast<domain::ChainId>(0)},
		  flag{static_cast<domain::ChainFlag>(0)},
		  player{domain::Player::FIRST}
	// card{},
	// effect{}
	{}

	ChainLink::ChainLink() noexcept = default;
	// 	: link_{}
	// {
	// 	//
	// }
}
