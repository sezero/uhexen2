void CheckAbilities() {
/* Pa3PyX:  Make this easier to control:
 * If the assassin is standing still for for some time (depending on her
 * clvl), and did not attack for the last 2 seconds, and the surrounding
 * light level is less than some threshold, set her abslight level to 0
 * and cloak. (The game uses model lightstyles to draw player model when
 * she is cloaking, so the exact number doesn't matter.) After that, for
 * each second of NOT standing still (or attacking, or walking into the
 * light), her abslight level is incremented at rate which again depends
 * on her clvl -- until it reaches 1, at which point she decloaks, or
 * until she becomes still again, where the invisibility is renewed.
 * The light threshold again depends on clvl -- the higher the clvl, the
 * higher the maximum light level at which she can cloak.
 */

float light_thr;
float declk_rate;
float clk_time;

	/* assassins only, and at least 3 years old. */
	if ((self.playerclass != CLASS_ASSASSIN) || (self.level < 3))
		return;

	/* clvl                    3    4    5    6    7    8    9   10+
	 * =============================================================
	 * Decloaking rate, 1/s    4    2 1.33    1  0.8 0.67 0.57  0.5
	 * Cloaking threshold     30   40   50   60   70   80   90  100
	 * Still time, s           9    8    7    6    5    4    3    2
	 * =============================================================
	 * A level 3 assassin can only cloak at light level 30, which is
	 * almost completely dark. Light level 100 is normal, everyday
	 * brightness. "It is rumored that Assassins can master this
	 * skill in broad daylight when their skills are all but perfected."
	 */

	declk_rate = 4.0 / (self.level - 2);
	light_thr = (self.level - 3) * 10 + 30;
	clk_time = 12 - self.level;

	/* cap constants at clvl 10 */
	if (declk_rate < 0.5)
		declk_rate = 0.5;
	if (light_thr > 100)
		light_thr = 100;
	if (clk_time < 2)
		clk_time = 2;

	/* Make sure that our attack active time (2.5s) is higher than the
	 * lowest decloaking time, so that an attack will ALWAYS give us away */
	if ((self.velocity == '0 0 0') && (self.last_attack < time - 2.5) &&
	    (self.light_level <= light_thr) &&
	   !(self.drawflags & MLS_POWERMODE))
	{
		if (!self.still_time) /* initialize still time */
		{
			self.still_time = time;
		}

		else if (time - self.still_time > clk_time)
		/* check if enough time passed standing still; cloak */
		{
			msg_entity = self;
			WriteByte(MSG_ONE, SVC_SET_VIEW_FLAGS);
			WriteByte(MSG_ONE, DRF_TRANSLUCENT);
			self.abslight = 0;
			self.drawflags (+) (DRF_TRANSLUCENT | MLS_ABSLIGHT);

			/* remember the time that we cloaked in the
			 * lip field (otherwise unused for players). */
			self.lip = time;
		}

		else
		/* if we are still cloaked but don't meet the still time
		 * because we were not standing still after we cloaked,
		 * then now that we are still again, we can reset our
		 * cloaking depletion indicator -- so that the assassin
		 * does not have to wait another still delay to make her
		 * next step. "It takes a few seconds for her to achieve
		 * this invisibility, but once attained, the illusion is
		 * perfect."  */
		{
			self.abslight = 0;
			self.lip = time;
		}
	}

	/* if we are already cloaked but not behaving: */
	else if (self.drawflags & (DRF_TRANSLUCENT | MLS_ABSLIGHT))
	{
		self.still_time = 0;
		self.abslight = (time - self.lip) * declk_rate;
		/* ready to decloak */
		if (self.abslight >= 1)
		{
			self.drawflags (-) (DRF_TRANSLUCENT | MLS_ABSLIGHT);
			self.abslight = 0;
			msg_entity = self;
			WriteByte(MSG_ONE, SVC_CLEAR_VIEW_FLAGS);
			WriteByte(MSG_ONE, DRF_TRANSLUCENT);
		}
	}

	/* if we are neither cloaked nor behaving: */
	else {
		self.still_time = 0;
	}
}
