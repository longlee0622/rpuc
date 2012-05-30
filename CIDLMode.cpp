
int CIDLChoice(bool rimState[],int base,int top,bool isRemap)
{
	int bestHeight=0;

	if(isRemap) return (top-base+1);	//�����������remapRCAֱ�ӵ�2D����

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
	if(bestHeight ==0 ) bestHeight = top-base+1;	//˵������û�ҵ�����Ľ⣬ֱ���õ�2D����û���remap������XXXXXXXXX��������bestHeight=0
	if ((top-base+1)%2 && bestHeight==(top-base+1)/2) return top-base+1;  //�����У��Ҹ߶ȴ���������һ�룬˫2Dƴ�Ӻ͵�2Dȡ��������һ���ģ�����OOOOXXO��˫2D�߶���4����2D�߶���7
	if ((top-base)%2 && bestHeight == (top-base+1)/2) return top-base+1;	//ż���У��Ҹ߶ȵ���������һ�룬˫2D�͵�2DЧ��һ�������ӣ�OXXOOXXO��˫2D�߶���4����2D�߶�8��Ч
	return bestHeight;

}
