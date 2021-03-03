#include <iostream>
#include <vector>
#include <string>
using namespace std;

void RemoveInternal(int descriptor, int lentodesc, int offset, int length);
string GetTextInternal(int descriptor, int lentodesc, int offset, int length);
struct PieceTableDescriptor
{
	bool Buffer;//false 为origin,true为modify
	int offset;
	int length;
};

string OriginalBuffer = "";
string ModifiedBuffer = "";

vector<PieceTableDescriptor> Descriptors;

int GetLength()//获取文本长度
{
	int length = 0;
	for (int i = 0; i < Descriptors.size(); i++)
	{
		length += ((PieceTableDescriptor)Descriptors[i]).length;
	}
	return length;
}

void Insert(int offset, string text)
{
	int len = 0; //起始位置
	int CurrentDesc = 0; //当前descriptor下标
	while (true)
	{
		PieceTableDescriptor desc = (PieceTableDescriptor)Descriptors[CurrentDesc]; //获取当前descriptor

		//判断当前offset是否在此descriptor中
		if ((len + desc.length) >= offset) //如果在
		{
			int gap = offset - len; //插入位置与该descriptor起始位置的间隔
			int newoffset = ModifiedBuffer.length(); //新的descriptor的起始位置

			//在buffer尾部添加文本
			ModifiedBuffer += text;

			//为新的文本设置descriptor
			PieceTableDescriptor newtext;
			newtext.offset = newoffset;
			newtext.length = text.length();
			newtext.Buffer = true;

			//插入位置是否在descriptor中间
			if (gap != 0 && gap != desc.length)//如果在
			{
				int end = desc.offset + desc.length;//截断后的结束位置

				desc.length = gap;//修改原descriptor的的长度为截断长度

				//为当前的descriptor后半段设置descriptor
				PieceTableDescriptor newdesc;
				newdesc.Buffer = desc.Buffer;
				newdesc.offset = desc.offset + desc.length;//起始位置为截断的位置
				newdesc.length = end - newdesc.offset;
				
				Descriptors[CurrentDesc] = desc;//将原有的descriptor替换为新的
				Descriptors.insert(Descriptors.begin() + 1 + CurrentDesc, newtext);//新文本和后半段的descriptor
				Descriptors.insert(Descriptors.begin() + 2 + CurrentDesc, newdesc);
			}

			else if (gap == desc.length)//在末尾
			{
				Descriptors.insert(Descriptors.begin() + 1 + CurrentDesc, newtext);
			}

			else  //在开头
			{
				Descriptors.insert(Descriptors.begin() + CurrentDesc,newtext);
			}

			break;
		}
		else
		{
			CurrentDesc++;
			len += desc.length;
			if (CurrentDesc == Descriptors.size())
				break;
		}
	}
}

void Remove(int offset, int length)
{
	int len = 0;
	int CurrentDesc = 0;
	while (true)
	{
		//当前descriptor是否包含offset
		if ((len + ((PieceTableDescriptor)Descriptors[CurrentDesc]).length) >= offset)
		{
			RemoveInternal(CurrentDesc, len, offset, length);
			break;
		}
		else
		{
			
			len += ((PieceTableDescriptor)Descriptors[CurrentDesc]).length;
			CurrentDesc++;
			if (CurrentDesc == Descriptors.size())
				break;
		}
	}
}

void RemoveInternal(int descriptor, int lentodesc, int offset, int length)
{
	PieceTableDescriptor desc = (PieceTableDescriptor)Descriptors[descriptor];
	int gap = offset - lentodesc;

	//移除的文本是否包含多个descriptors
	if ((offset + length) > (lentodesc + desc.length))
	{
		lentodesc += desc.length;
		length -= lentodesc - offset;
		offset = lentodesc;

		desc.length = gap;

		//要移除的文本是否包含全部的当前descriptor
		if (gap != 0) //不包含
		{
			Descriptors[descriptor] = desc;
			RemoveInternal(descriptor + 1, lentodesc, offset, length);

		}

		else
		{
			Descriptors.erase(Descriptors.begin() + descriptor - 1);
			RemoveInternal(descriptor, lentodesc, offset, length);
		}

	}

	else
	{
		//设置新的descriptor来映射被移除的文本
		PieceTableDescriptor newdesc;
		newdesc.Buffer = desc.Buffer;
		newdesc.offset = desc.offset + gap + length;
		newdesc.length = (desc.offset + desc.length) - newdesc.offset;

		desc.length = gap;

		//要移除的文本是否包含当前整个descriptor
		if (gap != 0) //不包含
		{
			Descriptors.insert(Descriptors.begin() + descriptor + 1, newdesc);
			Descriptors[descriptor] = desc; 
		}

		else
		{
			//直接将旧的设置为新的，而不是先删除后加入
			Descriptors[descriptor] = newdesc;
		}
	}
}

void Replace(int offset, int length, string text)
{
	Remove(offset, length);
	Insert(offset, text);
}

void SetText(string text)//打开文件时设置Original
{
	Descriptors.clear();
	ModifiedBuffer = "";
	OriginalBuffer = text;

	PieceTableDescriptor desc;
	desc.Buffer = false;
	desc.offset = 0;
	desc.length = text.length();

	Descriptors.push_back(desc);
}

char GetCharAt(int offset)
{
	int off = 0;
	int currdesc = 0;
	while (true)
	{
		PieceTableDescriptor desc = (PieceTableDescriptor)Descriptors[currdesc];

		//offset是否在当前descriptor
		if ((off + desc.length) > offset)
		{

			//找到descriptor开始处与offset的差值
			int gap = offset - off;

			if (desc.Buffer == false)
			{
				return OriginalBuffer[desc.offset + gap];
			}
			else 
			{
				return ModifiedBuffer[desc.offset + gap];
			}
		}

		else
		{
			off += desc.length;
		}

		currdesc++;
		if (currdesc == Descriptors.size())
			return '\0';
	}
}

string GetText(int offset, int length)
{
	string text = "";

	int off = 0;
	int currdesc = 0;

	while (true)
	{
		//当前descriptor是否包含坐标
		if ((off + ((PieceTableDescriptor)Descriptors[currdesc]).length) > offset)
		{
			//获取文本
			text += GetTextInternal(currdesc, off, offset, length);
			break;

		}

		else
		{
			currdesc++;
			off += ((PieceTableDescriptor)Descriptors[currdesc]).length;
			if (currdesc == Descriptors.size())
				break;
		}
	}
	return text;
}

string GetTextInternal(int descriptor, int lentodesc, int offset, int length)
{
	PieceTableDescriptor desc = (PieceTableDescriptor)Descriptors[descriptor];

	int gap = offset - lentodesc;

	string text = "";
	//所要文本是否大于当前descriptor
	if ((offset + length) > lentodesc + desc.length)
	{
		if (desc.Buffer)
		{
			text += ModifiedBuffer.substr(desc.offset + gap, desc.length);

		}

		else
		{
			text += OriginalBuffer.substr(desc.offset + gap, desc.length);

		}

		lentodesc += desc.length;
		length -= lentodesc - offset;
		offset = lentodesc;

		//从后一个descriptor中获取文本
		text += GetTextInternal(descriptor + 1, lentodesc, offset, length);
	}

	else
	{
		//所要文本在当前descriptor中
		if (desc.Buffer)
		{
			text += ModifiedBuffer.substr(desc.offset + gap, length);

		}

		else
		{
			text += OriginalBuffer.substr(desc.offset + gap, length);
		}
	}
	return text;
}

void OutPrint()
{
	int currdesc = 0;
	//cout << Descriptors.size()<<endl;
	//cout << Descriptors[0].Buffer<<endl;
	//cout << OriginalBuffer<<endl;
	PieceTableDescriptor desc;
	while (true)
	{
		
		desc = (PieceTableDescriptor)Descriptors[currdesc];
		if(Descriptors.size()>0)
		{
			if (desc.Buffer)
			{
				string textModified = ModifiedBuffer.substr(desc.offset, desc.length);
				cout << textModified;
			}

			else
			{
				string textOriginal = OriginalBuffer.substr(desc.offset, desc.length);
				cout << textOriginal;
			}
			currdesc++;
			if (currdesc == Descriptors.size())
				break;
		}
	}
}

int main()
{
	string t;
	cout << "输入文本:";
	cin >> t;
	SetText(t);
	//OutPrint();
	int off;
	string it;
	cout << "输入插入位置：";
	cin >> off;
	cout << "输入文本：";
	cin >> it;
	Insert(off, it);
	//cout << "yes";
	OutPrint();
	cout << endl;
	int del, l;
	cout << "删除文本位置：";
	cin >> del;
	cout << "删除长度：";
	cin >> l;
	Remove(del, l);
	OutPrint();
	cout << endl;
	int re, lre;
	string retext;
	cout << "替换文本位置：";
	cin >> re;
	cout << "替换文本长度：";
	cin >> lre;
	cout << "替换文本：";
	cin >> retext;
	Replace(re, lre, retext);
	OutPrint();
}