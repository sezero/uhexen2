void()player_necromancer_spell_attack;
void Nec_Spl_Fire (void)
{
	self.think=player_necromancer_spell_attack;
}

void spell_idle (void)
{
	self.th_weapon=spell_idle;
}

void spell_select (void)
{
	spell_idle();
}
