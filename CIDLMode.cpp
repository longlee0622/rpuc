
int CIDLChoice(bool rimState[],int base,int top,bool isRemap)
{
	int bestHeight=0;

	if(isRemap) return (top-base+1);	//简单起见，对于remapRCA直接单2D锁定

	bool occupy[32];
	for (int i = 0; i < 32; ++i) occupy[i]=rimState[i];
	for (int height = 1; height <= (top-base+1)/2; ++height)
	{
		bool match = true;
		for (int i = base+height;i <=top-height; ++i)
		{
			if (occupy[i] == true) 
			{
				match = false;
				break;
			}
		}
		if(match) 
		{
			bestHeight=height;
			break;
		}

	}
	if(bestHeight ==0 ) bestHeight = top-base+1;	//说明根本没找到满足的解，直接用单2D，最不济还有remap，例子XXXXXXXXX，遍历完bestHeight=0
	if ((top-base+1)%2 && bestHeight==(top-base+1)/2) return top-base+1;  //奇数行，且高度大于行数的一半，双2D拼接和单2D取出的数是一样的，例子OOOOXXO，双2D高度是4，单2D高度是7
	if ((top-base)%2 && bestHeight == (top-base+1)/2) return top-base+1;	//偶数行，且高度等于行数的一半，双2D和单2D效果一样，例子：OXXOOXXO，双2D高度是4，单2D高度8等效
	return bestHeight;

}
