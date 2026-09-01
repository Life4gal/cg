from _cg import PrototypeKind, CardType
import inspect

class _PrototypeBase:
	# 必须字段
	_required_fields = frozenset()
	# 可选字段
	_optional_fields = frozenset({"canonical_code", "series", "card_type"})
	# 禁止字段
	_forbidden_fields = frozenset()
	# 必要卡牌类型(如果子类没有定义则将其补上)
	_required_card_type = frozenset()

	@classmethod
	def _get_declared_fields(cls):
		"""返回类中定义的所有数据字段名(排除私有、方法和嵌套类)"""
		fields = set()
		for name, value in cls.__dict__.items():
			if name.startswith("_"):
				continue
			if inspect.isroutine(value) or inspect.isclass(value):
				continue
			fields.add(name)
		return fields


	@classmethod
	def _merge_inherited_constraints(cls):
		"""合并当前类和所有父类的约束集合"""
		merged = {
			"required": set(),
			"optional": set(),
			"forbidden": set(),
		}

		# 跳过自身,从父类开始遍历
		for base in cls.__mro__[1:]:
			if issubclass(base, _PrototypeBase):
				merged["required"].update(getattr(base, "_required_fields", set()))
				merged["optional"].update(getattr(base, "_optional_fields", set()))
				merged["forbidden"].update(getattr(base, "_forbidden_fields", set()))
		
		return merged

	@classmethod
	def _auto_fill_card_type(cls):
		"""自动补全或合并card_type,确保包含所有必需的卡牌类型"""
		required = getattr(cls, "_required_card_type", frozenset())
		if not required:
			return

		# 计算必需类型的掩码
		required_mask = CardType.NONE
		for ct in required:
			required_mask |= ct
		
		if hasattr(cls, "card_type"):
			user_ct = cls.card_type
			if not isinstance(user_ct, CardType):
				raise TypeError(f"[{cls.__name__}] 原型的 card_type 类型错误，实际为 {type(user_ct).__name__}")
			# 合并用户定义的和必要的
			cls.card_type = user_ct | required_mask
		else:
			# 使用必要的
			cls.card_type = required_mask

	@classmethod
	def _validate_fields(cls, merged, fields):
		"""检查是否存在禁止字段"""
		forbidden = merged["forbidden"]
		for name in fields:
			if name in forbidden:
				raise TypeError(f"[{cls.__name__}] 原型定义了禁止字段 {name!r}")

	@classmethod
	def _validate_required_fields(cls, merged, fields):
		"""检查所有必须字段是否都存在"""
		required = merged["required"]
		missing = sorted(required - fields)
		if missing:
			raise TypeError(f"[{cls.__name__}] 原型缺少了必要字段: {', '.join(repr(f) for f in missing)}")

	def __init_subclass__(cls, **kwargs):
		super().__init_subclass__(**kwargs)
		
		# 不校验本模块定义的基类
		if cls.__module__.split('.')[0] in ('cg'):
			return

		merged = cls._merge_inherited_constraints()
		# 自动补全/合并 card_type
		cls._auto_fill_card_type()
		# 获取当前类中定义的数据字段
		fields = cls._get_declared_fields()
		# 验证禁止字段和必须字段
		cls._validate_fields(merged, fields)
		cls._validate_required_fields(merged, fields)


# ==================== 怪兽 ====================

class MonsterPrototype(_PrototypeBase):
	# 原型类型是怪兽
	_prototype_kind = PrototypeKind.MONSTER

	# 必须字段
	_required_fields = frozenset({"attribute", "race", "level", "attack", "defense"})
	# 禁止字段
	# 普通怪兽没有阶级/连接箭头/灵摆刻度
	_forbidden_fields = frozenset({"rank", "link_marker", "left_pendulum", "right_pendulum"})
	# 必要卡牌类型
	_required_card_type = frozenset({CardType.MONSTER})
	

class RitualMonsterPrototype(MonsterPrototype):
	# 原型类型是仪式怪兽
	_prototype_kind = PrototypeKind.RITUAL_MONSTER

	# 必要卡牌类型
	_required_card_type = frozenset({CardType.MONSTER, CardType.RITUAL})


class FusionMonsterPrototype(MonsterPrototype):
	# 原型类型是融合怪兽
	_prototype_kind = PrototypeKind.FUSION_MONSTER

	# 必要卡牌类型
	_required_card_type = frozenset({CardType.MONSTER, CardType.FUSION})


class SynchroMonsterPrototype(MonsterPrototype):
	# 原型类型是同调怪兽
	_prototype_kind = PrototypeKind.SYNCHRO_MONSTER

	# 必要卡牌类型
	_required_card_type = frozenset({CardType.MONSTER, CardType.SYNCHRO})


class XyzMonsterPrototype(MonsterPrototype):
	# 原型类型是超量怪兽
	_prototype_kind = PrototypeKind.XYZ_MONSTER

	# 必须字段
	_required_fields = frozenset({"attribute", "race", "rank", "attack", "defense"})
	# 禁止字段
	# 超量怪兽没有等级/连接箭头/灵摆刻度
	_forbidden_fields = frozenset({"level", "link_marker", "link_marker", "left_pendulum", "right_pendulum"})
	# 必要卡牌类型
	_required_card_type = frozenset({CardType.MONSTER, CardType.XYZ})
	

class PendulumMonsterPrototype(MonsterPrototype):
	# 原型类型是灵摆怪兽
	_prototype_kind = PrototypeKind.PENDULUM_MONSTER

	# 必须字段
	_required_fields = frozenset({"attribute", "race", "level", "attack", "defense", "left_pendulum", "right_pendulum"})
	# 禁止字段
	# 灵摆怪兽没有阶级/连接箭头
	_forbidden_fields = frozenset({"rank", "link_marker"})
	# 必要卡牌类型
	_required_card_type = frozenset({CardType.MONSTER, CardType.PENDULUM})


class LinkMonsterPrototype(MonsterPrototype):
	# 原型类型是连接怪兽
	_prototype_kind = PrototypeKind.LINK_MONSTER

	# 必须字段
	_required_fields = frozenset({"attribute", "race", "link_marker", "attack"})
	# 禁止字段
	# 连接怪兽没有等级/阶级/防御力/灵摆刻度
	_forbidden_fields = frozenset({"level", "rank", "defense", "left_pendulum", "right_pendulum"})
	# 必要卡牌类型
	_required_card_type = frozenset({CardType.MONSTER, CardType.LINK})


# ==================== 魔法 / 陷阱 ====================

class _SpellTrapBasePrototype(_PrototypeBase):
	# 禁止字段
	# 魔陷没有属性/种族/等级/阶级/连接箭头/攻击力/防御力/灵摆刻度
	_forbidden_fields = frozenset({"attribute", "race", "level", "rank", "link_marker", "attack", "defense", "left_pendulum", "right_pendulum"})


class SpellPrototype(_SpellTrapBasePrototype):
	# 原型类型是魔法卡
	_prototype_kind = PrototypeKind.SPELL

	# 必要卡牌类型
	_required_card_type = frozenset({CardType.SPELL})


class TrapPrototype(_SpellTrapBasePrototype):
	# 原型类型是陷阱卡
	_prototype_kind = PrototypeKind.TRAP
	
	# 必要卡牌类型
	_required_card_type = frozenset({CardType.TRAP})

