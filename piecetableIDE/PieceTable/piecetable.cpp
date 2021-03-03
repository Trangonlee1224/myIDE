#include <iostream>
#include <vector>
#include <string>
using namespace std;

void RemoveInternal(int descriptor, int lentodesc, int offset, int length);
string GetTextInternal(int descriptor, int lentodesc, int offset, int length);
struct PieceTableDescriptor
{
	bool Buffer;//false Ϊorigin,trueΪmodify
	int offset;
	int length;
};

string OriginalBuffer = "";
string ModifiedBuffer = "";

vector<PieceTableDescriptor> Descriptors;

int GetLength()//��ȡ�ı�����
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
	int len = 0; //��ʼλ��
	int CurrentDesc = 0; //��ǰdescriptor�±�
	while (true)
	{
		PieceTableDescriptor desc = (PieceTableDescriptor)Descriptors[CurrentDesc]; //��ȡ��ǰdescriptor

		//�жϵ�ǰoffset�Ƿ��ڴ�descriptor��
		if ((len + desc.length) >= offset) //�����
		{
			int gap = offset - len; //����λ�����descriptor��ʼλ�õļ��
			int newoffset = ModifiedBuffer.length(); //�µ�descriptor����ʼλ��

			//��bufferβ������ı�
			ModifiedBuffer += text;

			//Ϊ�µ��ı�����descriptor
			PieceTableDescriptor newtext;
			newtext.offset = newoffset;
			newtext.length = text.length();
			newtext.Buffer = true;

			//����λ���Ƿ���descriptor�м�
			if (gap != 0 && gap != desc.length)//�����
			{
				int end = desc.offset + desc.length;//�ضϺ�Ľ���λ��

				desc.length = gap;//�޸�ԭdescriptor�ĵĳ���Ϊ�ضϳ���

				//Ϊ��ǰ��descriptor��������descriptor
				PieceTableDescriptor newdesc;
				newdesc.Buffer = desc.Buffer;
				newdesc.offset = desc.offset + desc.length;//��ʼλ��Ϊ�ضϵ�λ��
				newdesc.length = end - newdesc.offset;
				
				Descriptors[CurrentDesc] = desc;//��ԭ�е�descriptor�滻Ϊ�µ�
				Descriptors.insert(Descriptors.begin() + 1 + CurrentDesc, newtext);//���ı��ͺ��ε�descriptor
				Descriptors.insert(Descriptors.begin() + 2 + CurrentDesc, newdesc);
			}

			else if (gap == desc.length)//��ĩβ
			{
				Descriptors.insert(Descriptors.begin() + 1 + CurrentDesc, newtext);
			}

			else  //�ڿ�ͷ
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
		//��ǰdescriptor�Ƿ����offset
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

	//�Ƴ����ı��Ƿ�������descriptors
	if ((offset + length) > (lentodesc + desc.length))
	{
		lentodesc += desc.length;
		length -= lentodesc - offset;
		offset = lentodesc;

		desc.length = gap;

		//Ҫ�Ƴ����ı��Ƿ����ȫ���ĵ�ǰdescriptor
		if (gap != 0) //������
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
		//�����µ�descriptor��ӳ�䱻�Ƴ����ı�
		PieceTableDescriptor newdesc;
		newdesc.Buffer = desc.Buffer;
		newdesc.offset = desc.offset + gap + length;
		newdesc.length = (desc.offset + desc.length) - newdesc.offset;

		desc.length = gap;

		//Ҫ�Ƴ����ı��Ƿ������ǰ����descriptor
		if (gap != 0) //������
		{
			Descriptors.insert(Descriptors.begin() + descriptor + 1, newdesc);
			Descriptors[descriptor] = desc; 
		}

		else
		{
			//ֱ�ӽ��ɵ�����Ϊ�µģ���������ɾ�������
			Descriptors[descriptor] = newdesc;
		}
	}
}

void Replace(int offset, int length, string text)
{
	Remove(offset, length);
	Insert(offset, text);
}

void SetText(string text)//���ļ�ʱ����Original
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

		//offset�Ƿ��ڵ�ǰdescriptor
		if ((off + desc.length) > offset)
		{

			//�ҵ�descriptor��ʼ����offset�Ĳ�ֵ
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
		//��ǰdescriptor�Ƿ��������
		if ((off + ((PieceTableDescriptor)Descriptors[currdesc]).length) > offset)
		{
			//��ȡ�ı�
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
	//��Ҫ�ı��Ƿ���ڵ�ǰdescriptor
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

		//�Ӻ�һ��descriptor�л�ȡ�ı�
		text += GetTextInternal(descriptor + 1, lentodesc, offset, length);
	}

	else
	{
		//��Ҫ�ı��ڵ�ǰdescriptor��
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
	cout << "�����ı�:";
	cin >> t;
	SetText(t);
	//OutPrint();
	int off;
	string it;
	cout << "�������λ�ã�";
	cin >> off;
	cout << "�����ı���";
	cin >> it;
	Insert(off, it);
	//cout << "yes";
	OutPrint();
	cout << endl;
	int del, l;
	cout << "ɾ���ı�λ�ã�";
	cin >> del;
	cout << "ɾ�����ȣ�";
	cin >> l;
	Remove(del, l);
	OutPrint();
	cout << endl;
	int re, lre;
	string retext;
	cout << "�滻�ı�λ�ã�";
	cin >> re;
	cout << "�滻�ı����ȣ�";
	cin >> lre;
	cout << "�滻�ı���";
	cin >> retext;
	Replace(re, lre, retext);
	OutPrint();
}