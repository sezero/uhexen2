
void() SnakeHit =
{
        if(other==self.owner||(other.owner==self.owner&&other.classname=="snakearrow"))
                return;
        if(other.takedamage)
        {
                other.bloodloss=other.bloodloss+1;
                SpawnPuff(other.origin,'0 0 0',self.mass,other);
                T_Damage(other,self,self.owner,self.mass);
        }
        remove(self);
//        MultiExplode();
};

void() FireSnakeArrow =
{
local entity missile;
        missile=spawn();
        missile.classname="snakearrow";
        missile.movetype=MOVETYPE_FLYMISSILE;
        missile.solid=SOLID_BBOX;
//        missile.takedamage=DAMAGE_YES;
//        missile.health=10;
        if(self.classname=="player")
        {
                makevectors(self.v_angle);
                missile.owner=self;
                missile.mass=100;
                missile.aflag=TRUE;
                missile.o_angle=self.v_angle;
                self.attack_finished=time + 0.5;
        }
        else
        {
                makevectors(self.o_angle);
                missile.o_angle=self.o_angle;
        }
//        missile.th_die=MultiExplode;
        missile.touch=SnakeHit;

        missile.velocity=normalize(v_forward)*(350+self.mass);
        
//        setmodel(missile,"models/laser.mdl");
        missile.skin=2;
        setsize(missile,'0 0 0','0 0 0');
        if(self.classname=="snakearrow")
        {
                missile.owner=self.owner;
                missile.mass=self.mass=self.mass*0.5;
                setorigin(missile,self.origin);
                if(self.aflag)
                {
                        self.aflag=missile.aflag=FALSE;
						missile.velocity=missile.velocity+v_right*30;
						self.velocity=self.velocity-v_right*30;
				}
                else
                {
                        self.aflag=missile.aflag=TRUE;
						missile.velocity=missile.velocity+v_up*30;
						self.velocity=self.velocity-v_up*30;
                }
                if(self.mass>10)
                {
                        self.think=FireSnakeArrow;
                        self.nextthink=time + 0.15;
                }
                else
                {
                        self.think=SUB_Remove;
                        self.nextthink=time+5;
                }
        }
        else
                setorigin(missile,self.origin+self.proj_ofs+v_forward*8);

        missile.angles=vectoangles(missile.velocity);
        if(missile.mass>10)
        {
                missile.think=FireSnakeArrow;
                missile.nextthink=time + 0.15;
        }
        else
        {
                missile.think=SUB_Remove;
                missile.nextthink=time+5;
        }
};

