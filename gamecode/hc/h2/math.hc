/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/h2/math.hc,v 1.1.1.1 2004-11-29 11:39:05 sezero Exp $
 */



/*
 * crandom() -- Returns a random number between -1 and 1.
 */

float crandom()
{
	return random(-1,1);
}

float fexp(float base,float exponent)
{//MG
float exp_count;

	exponent=rint(exponent);
	if(exponent==0)
		return 1;
	if(exponent<0)
	{
		base=1/base;
		exponent=fabs(exponent);
	}

	if(exponent==1)
		return base;
	
	exponent-=1;
	while(exp_count<exponent)
	{
		exp_count+=1;
		base=base*base;
	}
	return base;
}

float byte_me(float mult)
{//MG
float mult_count,base;


	mult=rint(mult);
	if(mult==0)
		return 0;
	if(mult==1)
		return 1;
	if(mult==-1)
		return -1;

	if(mult<0)
	{
		base= -1;
		mult=fabs(mult);
	}
	else
		base=1;
	
	mult-=1;	
	while(mult_count<mult)
	{
		mult_count+=1;
		base=base*2;
	}
	return base;
}

vector RandomVector (vector vrange)
{
vector newvec;
	newvec_x=random(vrange_x,0-vrange_x);
	newvec_y=random(vrange_y,0-vrange_y);
	newvec_z=random(vrange_z,0-vrange_z);
	return newvec;
}

/*
 * $Log: not supported by cvs2svn $
 * 
 * 7     8/13/97 5:56p Mgummelt
 * 
 * 6     7/03/97 4:12p Mgummelt
 * 
 * 5     6/19/97 4:03p Rjohnson
 * Optimize crandom
 * 
 * 4     6/18/97 6:58p Mgummelt
 * 
 * 3     5/08/97 9:47p Mgummelt
 * 
 * 2     3/29/97 11:14a Aleggett
 * 
 * 1     3/21/97 9:23a Rlove
 */
