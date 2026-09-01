from cg import MonsterPrototype, CardType, Attribute, Race, Level

class Prototype(MonsterPrototype):
    #series = [Duel.register_series("测试卡牌")]
    card_type = CardType.MONSTER | CardType.EFFECT
    attribute = Attribute.LIGHT
    race = Race.CYBERSE
    level = Level.L4
    attack = 1500
    defense = 1000
