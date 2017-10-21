#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef struct{int w,d,h,m;}time;
int time2m(time t){
	return 60*(24*(7*t.w+t.d)+t.h)+t.m;
}
time m2time(int m){
	time t;
	t.m=m%60;
	m/=60;
	t.h=m%24;
	m/=24;
	t.d=m%7;
	m/=7;
	t.w=m;
	return t;
}

typedef struct{int id; time begin; int dur;} interval;
typedef struct ile{interval data; struct ile*next;} ile;
typedef struct{ile*startg; ile*endg;}intervallist;
intervallist nil(void){
	intervallist x;
	ile*startg=(ile*)malloc(sizeof(ile));
	ile*endg=(ile*)malloc(sizeof(ile));
	startg->next=endg;
	endg->next=NULL;
	x.startg=startg;
	x.endg=endg;
	return x;
}
ile*file2ile(ile*endg,FILE*fp){
	int tempint, flag=0;
	time temptime;
	if(!flag && fread(&tempint, sizeof(int), 1, fp)==1)(endg->data).id=tempint;
	else flag=1;
	if(!flag && fread(&temptime, sizeof(time), 1, fp)==1)(endg->data).begin=temptime;
	else flag=1;
	if(!flag && fread(&temptime, sizeof(time), 1, fp)==1)(endg->data).dur=time2m(temptime)-time2m((endg->data).begin);
	else flag=1;
	if (!flag){
		ile*nile=(ile*)malloc(sizeof(ile));
		nile->next=NULL;
		endg->next=nile;
		return file2ile(nile, fp);
	}
	else return endg;
}
int kphase (char*input, intervallist*il){
	ile*tempendg;
	FILE*fp=fopen(input, "rb");
	if (fp==NULL) return 1;
	tempendg=il->endg;
	il->endg=file2ile(tempendg, fp);
	if (il->endg==tempendg){
		if (!fclose(fp))return 3;
		else return 11;
	}
	if (!fclose(fp)) return 0;
	else return 2;
}

typedef struct{int id, dur, min, max, slack; time es, lf;}task;
typedef struct tle{task data; struct tle*next; struct tle*prev;}tle;
typedef struct{tle*startg; tle*endg;}tasklist;
typedef struct{int id, h, m, minh, minm, maxh, maxm;}tgn;
typedef struct tgnle{tgn node; int parentid; struct tgnle*next;}tgnle;
tasklist ntl(void){tasklist x;
tle*startg=(tle*)malloc(sizeof(tle));
tle*endg=(tle*)malloc(sizeof(tle));
startg->next=endg;
endg->prev=startg;
endg->next=NULL;
x.startg=startg;
x.endg=endg;
return x;
}
int helper_file2tgnle(FILE*fp, tgnle*listend){
	int tempint;
	tgnle*result=(tgnle*)malloc(sizeof(tgnle));
	result->next=NULL;
	listend->next=result;
	if (fread(&tempint, sizeof(int), 1, fp)==1)
		(result->node).id=tempint;
	else {
		return 1;
	}
	if (fread(&tempint, sizeof(int), 1, fp)==1)
		(result->node).h=tempint;
	else {
		return 2;
	}
	if (fread(&tempint, sizeof(int), 1, fp)==1){
		if (tempint<60) (result->node).m=tempint;
		else {
			return 3;
		}
	}
	else {
		return 2;
	}
	if (fread(&tempint, sizeof(int), 1, fp)==1)
		(result->node).minh=tempint;
	else {
		return 2;
	}
	if (fread(&tempint, sizeof(int), 1, fp)==1){
		if (tempint<60) (result->node).minm=tempint;
		else {
			return 3;
		}
	}
	else {
		return 2;
	}
	if (fread(&tempint, sizeof(int), 1, fp)==1)
		(result->node).maxh=tempint;
	else {
		return 2;
	}
	if (fread(&tempint, sizeof(int), 1, fp)==1){
		if (tempint<60) (result->node).maxm=tempint;
		else {
			return 3;
		}
	}
	else {
		return 2;
	}
	if (fread(&tempint, sizeof(int), 1, fp)==1)
		result->parentid=tempint;
	return 0;
}
void helper_dfs(tle*startg, tgnle*p){
	tle*ntle=(tle*)malloc(sizeof(tle));
	(ntle->data).id=(p->node).id;
	(ntle->data).dur=(60*(p->node).h+(p->node).m);
	(ntle->data).min=(60*(p->node).minh+(p->node).minm);
	(ntle->data).max=(60*(p->node).maxh+(p->node).maxm);
	startg->next->prev=ntle;
	ntle->prev=startg;
	ntle->next=startg->next;
	startg->next=ntle;
}
void dfs(tgnle*startg, tgnle*endg, int root, tasklist*tl){
	tgnle*p=startg;
	do {
		p=p->next;
		if (p->parentid==root) {
			int flag=0;
			tle*q=tl->startg;
			if (tl->startg->next==tl->endg) flag=0;
			else {
				do {
					q=q->next;
					if ((q->data).id==(p->node).id)flag=1;
				} while (flag!=1 && q->next!=tl->endg);
			}
			if (!flag){
				dfs(startg, endg, (p->node).id, tl);
				helper_dfs(tl->startg, p);
			}
		}
	}
	while (p->next!=endg);
}
int file2tle(tasklist*tl,tgnle*startg,FILE*fp){
	enum readerr {
		NO_ERROR,
		READ_ERROR,
		FORMAT_ERROR
	} result;
	int errcode;
	tgnle*endg=startg;
	errcode=helper_file2tgnle(fp, endg);
	if (errcode==1) return 1;
	while(errcode==0){
		endg=endg->next;
		errcode=helper_file2tgnle(fp, endg);
	}
	endg=endg->next;
	endg->next=NULL;
	result=errcode-1;
	if (result==NO_ERROR)dfs(startg, endg, 0, tl);
	switch (result) {
	case NO_ERROR:
		return 0;
	case READ_ERROR:
		return 1;
	case FORMAT_ERROR:
		return 2;
	default:
		return 3;
	}
}
int ephase(char*input,tasklist*tl){
	FILE*fp=fopen(input, "rb");
	if (fp==NULL) return 11;
	else{
		tgnle*startg=(tgnle*)malloc(sizeof(tgnle));
		int errcode=file2tle(tl, startg, fp);
		if (!errcode && tl->startg->next->next!=tl->endg) {
			tle*p=tl->startg;
			do {
				time mef=m2time(0);
				tgnle*q=startg;
				p=p->next;
				do {
					q=q->next;
					if ((q->node).id==(p->data).id) {
						int temp=q->parentid;
						tle*r=tl->startg;
						do {
							r=r->next;
							if ((r->data).id==temp && time2m((r->data).es)+(r->data).dur>time2m(mef)) mef=m2time(time2m((r->data).es)+(r->data).dur);
						} while (r!=p);
					}
				} while (q->next->next!=NULL);
				(p->data).es=mef;
			} while (p->next!=tl->endg);
			(p->data).lf=m2time(time2m((p->data).es)+(p->data).dur);
			do {
				time mls=(tl->endg->prev->data).lf;
				tgnle*q=startg;
				p=p->prev;
				do {
					q=q->next;
					if (q->parentid==(p->data).id) {
						int temp=(q->node).id;
						tle*r=tl->endg;
						do {
							r=r->prev;
							if ((r->data).id==temp && time2m((r->data).lf)-(r->data).dur<time2m(mls)) mls=m2time(time2m((r->data).lf)-(r->data).dur);
						} while (r!=p);
					}
				} while (q->next->next!=NULL);
				(p->data).lf=mls;
			} while (p->prev!=tl->startg);
			do {
				(p->data).slack=time2m((p->data).lf)-time2m((p->data).es)-(p->data).dur;
				p=p->next;
			} while (p->next!=tl->endg);
			(p->data).slack=time2m((p->data).lf)-time2m((p->data).es)-(p->data).dur;
			do {
				tle*maximum[1]={p};
				tle*q=tl->startg;
				do {
					q=q->next;
					if (time2m((q->data).es)>time2m(((*maximum)->data).es)) *maximum=q;
				} while (q->next!=p);
				if (*maximum!=p) {
					p->next->prev=(*maximum);
					(*maximum)->next->prev=(*maximum)->prev;
					(*maximum)->prev->next=(*maximum)->next;
					(*maximum)->prev=p;
					(*maximum)->next=p->next;
					p->next=(*maximum);
				}
				else {
					p=p->prev;
				}
			} while (p->prev!=tl->startg);
		}
		{
			tgnle*p=startg;
			do {
				tgnle*temp=p;
				p=p->next;
				free(temp);
			} while (p!=NULL);
			if (tl->startg->next->next==tl->endg){
				if (!fclose(fp)) return 2;
				else return 13;
			}
			if (!fclose(fp)) return errcode;
			return 12;
		}
	}
}

typedef struct{int*members; time begin; int dur; int taskid;}fdata;
typedef struct fle{fdata data; struct fle*next;}fle;
typedef struct{fle*startg; fle*endg; int nom;}flist;
flist nfl(int nom){
	flist x;
	fle*startg=(fle*)malloc(sizeof(fle));
	fle*endg=(fle*)malloc(sizeof(fle));
	startg->next=endg;
	(endg->data).dur=0;
	(endg->data).begin=m2time(0);
	(startg->data).dur=0;
	(startg->data).begin=m2time(0);
	(startg->data).taskid=-1;
	endg->next=NULL;
	x.startg=startg;
	x.endg=endg;
	x.nom=nom;
	return x;
}
fle*helper_ile2fle(fle*endg, int timeleft, int*memberids, int nom, int id){
	int i;
	fle*nendg=(fle*)malloc(sizeof(fle));
	endg->next=nendg;
	nendg->next=NULL;
	(nendg->data).dur=0;
	(nendg->data).begin=(endg->data).begin;
	(endg->data).members=(int*)malloc(nom*sizeof(int));
	for (i=0; i<nom; i++) {
		if (memberids[i]==id) *((endg->data).members+i)=1;
		else *((endg->data).members+i)=0;
	}
	(endg->data).begin=m2time(time2m((endg->data).begin)-timeleft);
	(endg->data).dur=timeleft;
	return nendg;
};
void ile2fle(flist*fl, ile*latest, int*memberids){
	int i;
	int timeleft;
	fle*temp;
	fle*nfle=(fle*)malloc(sizeof(fle));
	nfle->next=fl->startg->next;
	fl->startg->next=nfle;
	(fl->startg->data).dur=time2m((latest->data).begin);
	if (((fl->startg->data).dur+(latest->data).dur)>time2m((fl->endg->data).begin))(fl->endg->data).begin=m2time((fl->startg->data).dur+(latest->data).dur);
	(nfle->data).members=(int*)malloc((fl->nom)*sizeof(int));
	for (i=0; i<fl->nom; i++) {
		if (memberids[i]==(latest->data).id) *((nfle->data).members+i)=1;
		else *((nfle->data).members+i)=0;
	}
	(nfle->data).begin=(latest->data).begin;
	(nfle->data).dur=time2m((nfle->next->data).begin)-time2m((nfle->data).begin);
	timeleft=(latest->data).dur;
	temp=nfle;
	while (timeleft>0) {
		if (temp==fl->endg) {
			fl->endg=helper_ile2fle(fl->endg, timeleft, memberids, (fl->nom), (latest->data).id);
			timeleft=0;
		}
		else {
			if ((temp->data).dur>timeleft) {
				fle*ntemp=(fle*)malloc(sizeof(fle));
				ntemp->next=temp->next;
				temp->next=ntemp;
				(ntemp->data).members=(int*)malloc((fl->nom)*sizeof(int));
				for (i=0; i<fl->nom; i++) *((ntemp->data).members+i)=*((temp->data).members+i);
				if (timeleft!=(latest->data).dur) {
					for (i=0; i<fl->nom; i++)if (memberids[i]==(latest->data).id) *((temp->data).members+i)=1;
				}
				else {
					for (i=0; i<fl->nom; i++) *((ntemp->data).members+i)=0;
					(ntemp->data).taskid=-3;
				}
				(ntemp->data).begin=m2time(time2m((temp->data).begin)+timeleft);
				(ntemp->data).dur=(temp->data).dur-timeleft;
				(temp->data).dur=timeleft;
				timeleft=0;
			}
			else {
				for (i=0; i<fl->nom; i++)if (memberids[i]==(latest->data).id) *((temp->data).members+i)=1;
				timeleft-=(temp->data).dur;
				temp=temp->next;
			}
		}
	}
}
void helper_fphase(fle*startg, int*memberids, int nom){
	FILE*fp=fopen("output.txt", "w");
	fle*p=startg;
	int i;
	do {
		p=p->next;
		for (i=0; i<nom; i++)if (*((p->data).members+i)==1) fprintf(fp, "%d, ", memberids[i]);
		fprintf(fp, "\t%d:%d:%d:%d \t%d\t%d\n", ((p->data).begin).w, ((p->data).begin).d, ((p->data).begin).h, ((p->data).begin).m,  (p->data).dur, (p->data).taskid);
	} while (p->next->next!=NULL && (p->next->data).taskid!=-2);
}
void splitting (fle*p, int nom, int split){
	int i;
	fle*ntemp=(fle*)malloc(sizeof(fle));
	ntemp->next=p->next;
	p->next=ntemp;
	(ntemp->data).members=(int*)malloc(nom*sizeof(int));
	for (i=0; i<nom; i++) *((ntemp->data).members+i)=*((p->data).members+i);
	(ntemp->data).dur=(p->data).dur-split;
	(ntemp->data).begin=m2time(time2m((p->data).begin)+split);
	(ntemp->data).taskid=-1;
	(p->data).dur=split;
}

int max_(tasklist*tl,int taskid){
	tle*p=tl->startg;
	do {
		p=p->next;
		if ((p->data).id==taskid) return (p->data).max;
	} while (p->next!=tl->endg);
	return -1;
}
void completion(fle* p, int taskid, tasklist*tl, int nom){
	tle*completing, *prev;
	int noam, i, split;
	(p->data).taskid=taskid;
	completing=tl->startg;
	do {
		prev=completing;
		completing=completing->next;
	} while ((completing->data).id!=taskid);
	noam=0, split=0;
	for (i=0; i<nom; i++) if (*((p->data).members+i)==1) noam++;
	if (noam==0) return;
	if ((completing->data).dur-noam*((p->data).dur)<0){
		if ((completing->data).dur-noam*((completing->data).min)<0)split=(completing->data).min;
		else {
			if (((completing->data).dur)%noam==0) split=((completing->data).dur)/noam;
			else split=((completing->data).dur)/noam+1;
		}
	}
	else {
		if ((p->data).dur>=(completing->data).min) {
			if (((completing->data).dur)%noam==0) split=((completing->data).dur)/noam;
			else split=((completing->data).dur)/noam+1;
		}
	}
	if (split<(p->data).dur){
	if (split==0) (p->data).taskid=-1;
	else splitting(p, nom, split);
	}
	else split=(p->data).dur;
	if ((completing->data).dur<=noam*split) {
		completing->next->prev=prev;
		prev->next=completing->next;
		free(completing);
	}	
	else (completing->data).dur-=noam*split;
}
int nexttaskid(int taskid, tasklist*tl){
	tle*next=tl->startg;
	int found=1;
	do {
		tle*i;
		next=next->next;
		i=tl->startg;
		do {
			i=i->next;
			if(time2m((i->data).lf)<time2m((next->data).es)) found=0;
		} while (found==1 && i->next!=tl->endg);
		if(found) {
			if ((next->data).id==taskid) return (next->next->data).id;
				else return (next->data).id;
		}
	} while (next->next!=tl->endg);
	return -1;
}
int fphase (flist*fl, intervallist*il ,tasklist*tl, int*memberids){
	int status=0, flag;
	fle*p, *r;
	do {
		ile*temp=il->startg;
		ile*latest=temp->next;
		do {
			temp=temp->next;
			if (time2m((temp->data).begin)>time2m((latest->data).begin)) latest=temp;
		} while (temp->next!=il->endg);
		ile2fle(fl, latest, memberids);
		temp=il->startg;
		do {
			temp=temp->next;
		} while (temp->next!=latest && temp!=latest);
		if (temp->next==latest)temp->next=latest->next;
		else il->startg->next=il->endg;
		free(latest);
	} while (il->startg->next!=il->endg);
	free(il->startg);
	free(il->endg);
	flag=0;
	p=fl->startg;
	do {
		fle*prev=p;
		p=p->next;
		if ((p->data).taskid!=-3){
		if ((prev->data).taskid==-1)completion(p,(tl->startg->next->data).id, tl, fl->nom);
		else {
			flag+=(prev->data).dur;
			if (flag<max_(tl, (prev->data).taskid)) {
				if (flag+(p->data).dur>max_(tl, (prev->data).taskid)) {
					int split=max_(tl, (prev->data).taskid)-flag;
					splitting (p, fl->nom, split);
				}
				completion(p,(prev->data).taskid, tl, fl->nom);
			}
			else {
				int nextid=nexttaskid((prev->data).taskid, tl);
				if (nextid!=-1) completion(p,nextid, tl, fl->nom);
				else (p->data).taskid=-1;
				flag=0;
			}
		}
		}
	} while (tl->startg->next!=tl->endg && p->next!=fl->endg);
	if (tl->startg->next==tl->endg) {
		(p->next->data).taskid=-2;
		free(tl->startg);
		free(tl->endg);
	}
	else {
		tle*q=tl->startg;
		do {
			tle*temp=q;
			q=q->next;
			free(temp);
		} while (q!=NULL);
		status=1;
	}
	helper_fphase(fl->startg, memberids, fl->nom);
	r=fl->startg->next;
	do {
		fle*temp=r;
		r=r->next;
		free((temp->data).members);
		free(temp);
	} while (r!=fl->endg);
	free(fl->startg);
	free(fl->endg);
	return status;
}

void buildt(void){
	int i, j[]={1,1,0,1,0,1,0,0,2,1,0,1,0,1,0,0,3,1,0,1,0,1,0,1,4,1,0,0,10,1,0,1,4,1,0,0,10,1,0,2,5,1,0,1,0,1,0,2,5,1,0,1,0,1,0,4,5,1,0,1,0,1,0,3,-1};
	FILE*fp=fopen("tasks", "wb");
	for (i=0; j[i]!=-1; i++) fwrite(j+i, sizeof(int), 1, fp);
	fclose(fp);
}
void buildi(void){
	int i, j[]={2,0,1,13,0,0,1,15,51,1,0,1,15,40,0,1,16,50,3,0,2,8,0,0,2,11,0,-1};
	FILE*fp=fopen("intervals", "wb");
	for (i=0; j[i]!=-1; i++)	fwrite(j+i, sizeof(int), 1, fp);
	fclose(fp);
}

int main (void){
	FILE*ffp;
	char s[21];
	char tinput[21];
	char group[21];
	char iinput[21];
	char puffer[41];
	char*c = puffer;
	int nom, i, istatus, tstatus;
	int*memberids;
	time deadline;
	intervallist il=nil();
	tasklist tl=ntl();
	flist fl;
	printf("Üdvözöllek! Tesztüzemmódban vagyunk, bemenetért nézd meg a t.txt fájlt, további infóért a kódot.\n");
	buildt();
	buildi();
	ffp=fopen("t.txt", "r");
	if (ffp==NULL)return 1;
	fgets(s, 21, ffp);
	fscanf(ffp, "%d%d%d%d", &deadline.w, &deadline.d, &deadline.h, &deadline.m);
	fgets(tinput, 21, ffp);
	fgets(tinput, 21, ffp);
	tinput[strlen(tinput)-1] = 0;
	fgets(group, 21, ffp);
	fgets(iinput, 21, ffp);
	iinput[strlen(iinput)-1] = 0;
	fscanf(ffp, "%d\n", &nom);
	fl=nfl(nom);
	memberids=(int*)malloc(nom*sizeof(int));
	*memberids=0;
	for (i=0; i<nom; i++){
		fgets(c, 41, ffp);
		while (*c!='\t') c++;
		*(memberids+i)=0;
		do {
			*(memberids+i)*=10;
			c++;
			if (*c<'0' || *c>'9'){
				*(memberids+i)/=10;
				break;
			}
			*(memberids+i)+=*c-'0';
		} while (1);
	}
	printf("A projekt neve: %s\b\nhatáridő: %d:%d:%d:%d\ncsapat: %s\b\t(%d)\n%s\t%s\n", s, deadline.w, deadline.d, deadline.h, deadline.m, group, nom, tinput, iinput);
	istatus=kphase(iinput, &il), tstatus=ephase(tinput, &tl);
	if (!istatus) printf("K fázis sikeres.\n");
	else printf("K fázisban hiba: %d.\n", istatus);
	if (!tstatus) printf("E fázis sikeres.\n");
	else printf("E fázisban hiba: %d.\n", tstatus);
	if (istatus==0 && tstatus==0) {
		int fstatus=fphase(&fl, &il, &tl, memberids);
		fle*last=fl.startg;
		do {
			last=last->next;
		} while (last->next!=fl.endg && (last->next->data).taskid!=-2);
		if (!fstatus && time2m((last->data).begin)+(last->data).dur<=time2m(deadline)) printf("A projekt elkészül határidőre.\n");
		else printf("A projekt nem készül el határidőre.\n");
	}
	else {
		tle*tp=tl.startg;
		ile*ip=il.startg;
		free(fl.startg);
		free(fl.endg);
		do {
			ile*temp=ip;
			ip=ip->next;
			free(temp);
		} while (ip!=NULL);
		tp=tl.startg;
		do {
			tle*temp=tp;
			tp=tp->next;
			free(temp);
		} while (tp!=NULL);
	}
	free(memberids);
	if(fclose(ffp)) return 1;
	return 0;
}