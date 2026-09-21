/* Opt-in controller for the corrected reward route. It reads gameplay state
 * and emits normal controller buttons only; no gameplay RAM is modified.
 * Prefix/navigation remain the canonical route inputs. */
static uint16_t RoutePilotAimPad(int dx, int dz)
{
    int c=(int16_t)PE_LoadU16(0x800BD000u),sn=(int16_t)PE_LoadU16(0x800BD004u);
    int x,z;
    uint16_t mask=0xFFFFu;
    if (!c && !sn) { x=dx; z=dz; }
    else { x=c*dx-sn*dz; z=sn*dx+c*dz; }
    if ((int64_t)dx*dx+(int64_t)dz*dz>6400) {
        if (abs(x)*2>abs(z)) mask&=x>0?0xFFDFu:0xFF7Fu;
        if (abs(z)*2>abs(x)) mask&=z>0?0xFFEFu:0xFFBFu;
    }
    return mask;
}

static uint16_t RouteRewardSewerPilot(uint16_t mask)
{
    static int heal_start=-1,equip_start=-1,equip_to=2,m31_stage=0,m32_stage=0;
    int m34=(GA_TOKEN==0xA8003248u);
    if (GA_TOKEN!=0xA80030C8u) m31_stage=0;
    if (GA_TOKEN!=0xA8003148u) m32_stage=0;
    /* M34 must own the pad from room entry, not only after the battle flag.
     * The recorded supply suffix is Cross/idle, so leaving it in control
     * parks Aya on the alligator's charge line. */
    if ((GA_TOKEN==0xA80023C8u || GA_TOKEN==0xA8002448u || GA_TOKEN==0xA8003148u || m34) &&
        (m34 || (PE_LoadU32(0x8009D1A0u)&2u))) {
        pe_addr_t aya=PE_LoadU32(0x8009D254u),rec=PE_LoadU32(0x8009D278u);
        unsigned mode=PE_LoadU32(0x8009D28Cu);
        mask=0xFFFFu;
        if (m34 && aya && rec && mode!=3u && mode!=9u) {
            pe_addr_t gun=PE_LoadU32(rec+104u),head=0,tail=0;
            int ax=(int32_t)PE_LoadU32(aya+40u)>>16,az=(int32_t)PE_LoadU32(aya+48u)>>16;
            unsigned loaded=gun?(PE_LoadU32(gun+12u)&0x3FFu):0u;
            unsigned hp=PE_LoadU16(rec+12u);
            int pocket=0,h2=0,t2=0,hx=0,hz=0,tx=0,tz=0;
            for (unsigned i=0;i<45;i++) {
                pe_addr_t p=PE_LoadU32(0x8009E000u+i*12u);
                unsigned type;
                if (!p) break;
                if (p==aya || !PE_RangeIsRam(p,640u) || !PE_RangeIsRam(PE_LoadU32(p),24u) ||
                    (int32_t)PE_LoadU32(PE_LoadU32(p)+16u)<=0) continue;
                type=PE_LoadU8(p+12u);
                if (type==3u) head=p;
                else if (type==4u) tail=p;
                else if (!head) head=p;
            }
            if (head) {
                hx=(int32_t)PE_LoadU32(head+40u)>>16; hz=(int32_t)PE_LoadU32(head+48u)>>16;
                h2=(hx-ax)*(hx-ax)+(hz-az)*(hz-az);
            }
            if (tail) {
                tx=(int32_t)PE_LoadU32(tail+40u)>>16; tz=(int32_t)PE_LoadU32(tail+48u)>>16;
                t2=(tx-ax)*(tx-ax)+(tz-az)*(tz-az);
            }
            pocket=tail && t2>=250000 && t2<=1000000 && h2>=900000;
            if (mode==1u) {
                unsigned selected=PE_LoadU8(0x8009CE44u),count=PE_LoadU8(0x8009D2B0u),target=selected,j;
                for (j=0;j<count;j++) {
                    pe_addr_t p=PE_LoadU32(0x8009E000u+j*12u);
                    if (p && PE_LoadU8(p+12u)==4u) { target=j; break; }
                }
                if (hp<=30u || !pocket || !loaded || heal_start>=0) {
                    if (g_frame%8==3) mask=0xDFFFu;
                } else if (g_frame%16==3) mask=target!=selected?0xFFEFu:0xBFFFu;
            } else if (!head) {
                mask=RoutePilotAimPad(-2400-ax,900-az);
            } else {
                int hdx=hx-ax,hdz=hz-az,tdx=tail?tx-ax:hdx,tdz=tail?tz-az:hdz;
                int want_x,want_z;
                if (h2<1600000) {
                    want_x=ax-hx; want_z=az-hz;
                    if (tail && want_x*tdx+want_z*tdz<0) { int s=want_x; want_x=want_z; want_z=-s; }
                } else if (!tail || t2>1000000) {
                    if (h2<t2 && hdx*tdx+hdz*tdz>0) {
                        want_x=-hdz; want_z=hdx;
                        if (tail && want_x*(tx-hx)+want_z*(tz-hz)<0) { want_x=-want_x; want_z=-want_z; }
                    } else { want_x=tdx; want_z=tdz; }
                } else if (t2<250000) { want_x=-tdx; want_z=-tdz; }
                else { want_x=tdz; want_z=-tdx; }
                mask=RoutePilotAimPad(want_x,want_z);
                if (pocket && loaded && heal_start<0 && g_frame%24==3) mask&=0xBFFFu;
            }
            if (heal_start<0 && equip_start<0 && mode==0u && !PE_LoadU8(0x8009CE3Cu) &&
                hp<=30u && PE_LoadU16(rec+16u)>=9000u && PE_LoadU32(rec+8u)>=60u*65536u) {
                heal_start=g_frame;fprintf(stderr,"LOOT_PILOT_HEAL %d\n",g_frame);
            }
            if (g_frame%30==0) {
                fprintf(stderr,"M34_PILOT %d mode=%u pad=%04X pos=%d,%d head=%d,%d tail=%d,%d hp=%u loaded=%u pocket=%d h2=%d t2=%d\n",
                        g_frame,mode,mask,ax,az,hx,hz,tx,tz,hp,loaded,pocket,h2,t2);
            }
        } else if (!mode && aya && rec) {
            pe_addr_t enemy=0;
            int nearest=INT32_MAX;
            for (unsigned i=0;i<45;i++) {
                pe_addr_t p=PE_LoadU32(0x8009E000u+i*12u);
                if (!p) break;
                if (PE_RangeIsRam(p,640u) && p!=aya && PE_RangeIsRam(PE_LoadU32(p),24u) &&
                    (int32_t)PE_LoadU32(PE_LoadU32(p)+16u)>0) {
                    if (GA_TOKEN!=0xA8003148u) {enemy=p;break;}
                    int dx=((int32_t)PE_LoadU32(p+40u)>>16)-((int32_t)PE_LoadU32(aya+40u)>>16);
                    int dz=((int32_t)PE_LoadU32(p+48u)>>16)-((int32_t)PE_LoadU32(aya+48u)>>16);
                    int distance=dx*dx+dz*dz;
                    if (distance<nearest) {nearest=distance;enemy=p;}
                }
            }
            if (enemy) {
                int dx=((int32_t)PE_LoadU32(enemy+40u)>>16)-((int32_t)PE_LoadU32(aya+40u)>>16);
                int dz=((int32_t)PE_LoadU32(enemy+48u)>>16)-((int32_t)PE_LoadU32(aya+48u)>>16);
                if (GA_TOKEN==0xA8003148u && PE_LoadU8(0x800C0E20u)==0u) {
                    int distance=dx*dx+dz*dz;
                    if (distance<640000) {dx=-dx;dz=-dz;}
                    else if (distance<902500) {int old=dx;dx=dz;dz=-old;}
                    int ax=(int32_t)PE_LoadU32(aya+40u)>>16,az=(int32_t)PE_LoadU32(aya+48u)>>16;
                    if (ax<-16000) dx=600;else if (ax>-14000) dx=-600;
                    if (az<1500) dz=600;else if (az>2700) dz=-600;
                }
                int c=(int16_t)PE_LoadU16(0x800BD000u),sn=(int16_t)PE_LoadU16(0x800BD004u);
                int x=c*dx-sn*dz,z=sn*dx+c*dz;
                if (dx*dx+dz*dz>6400) {
                    if (abs(x)*2>abs(z)) mask&=x>0?0xFFDFu:0xFF7Fu;
                    if (abs(z)*2>abs(x)) mask&=z>0?0xFFEFu:0xFFBFu;
                }
            }
            if (heal_start<0 && equip_start<0 && !PE_LoadU8(0x8009CE3Cu) && PE_LoadU16(rec+12u)<=25u &&
                PE_LoadU16(rec+16u)>=9000u && PE_LoadU32(rec+8u)>=60u*65536u) {
                heal_start=g_frame;fprintf(stderr,"LOOT_PILOT_HEAL %d\n",g_frame);
            }
        }
        pe_addr_t gun=rec?PE_LoadU32(rec+104u):0u;
        if (GA_TOKEN==0xA8003148u && equip_start<0 && heal_start<0 && !mode && rec &&
            PE_LoadU8(0x800C0E20u)==2u && !PE_LoadU8(0x8009CE3Cu) &&
            PE_LoadU16(0x800A1E6Eu)>0u && PE_LoadU16(rec+16u)>=9000u) {
            equip_to=0;equip_start=g_frame;
            fprintf(stderr,"LOOT_PILOT_PISTOL %d\n",g_frame);
        }
        if (equip_start<0 && heal_start<0 && !mode && rec && gun &&
            PE_LoadU8(0x800C0E20u)==0u && !PE_LoadU8(0x8009CE3Cu) &&
            !(PE_LoadU32(gun+12u)&0x3FFu) && !PE_LoadU16(0x800A1E6Eu) && PE_LoadU16(rec+16u)>=9000u) {
            equip_to=2;equip_start=g_frame;fprintf(stderr,"LOOT_PILOT_EQUIP %d\n",g_frame);
        }
        if (equip_start>=0 && PE_LoadU8(0x8009CE3Cu)) {
            fprintf(stderr,"LOOT_PILOT_EQUIP_QUEUED %d command=%u slot=%u\n",g_frame,PE_LoadU16(0x800BE834u),PE_LoadU8(0x800C0E20u));equip_start=-1;
        }
        if (heal_start>=0 && PE_LoadU8(0x8009CE3Cu)) heal_start=-1;
        if (equip_start>=0) {
            pe_addr_t focus=PE_LoadU32(0x8009D15Cu);
            unsigned id=focus?PE_LoadU32(focus+36u):0xFFFFFFFFu;
            unsigned row=focus?PE_LoadU32(focus+72u):0u;
            mask=0xFFFFu;
            if (g_frame-equip_start<2) mask=0xEFFFu;
            else if (g_frame%16==3 && focus) {
                unsigned target_row=id==7?(equip_to==2?1u:0u):2u;
                if ((id==0 || id==7) && row<target_row) mask=0xFFBFu;
                else if ((id==0 || id==7) && row>target_row) mask=0xFFEFu;
                else mask=0xBFFFu;
            }
        } else if (heal_start>=0) {
            pe_addr_t focus=PE_LoadU32(0x8009D15Cu);
            unsigned id=focus?PE_LoadU32(focus+36u):0xFFFFFFFFu;
            mask=0xFFFFu;
            if (g_frame-heal_start<2) mask=0xEFFFu;
            else if (g_frame%16==3 && focus) {
                unsigned row=PE_LoadU32(focus+72u);
                if (GA_TOKEN!=0xA8003148u && GA_TOKEN!=0xA8003248u) {
                    if (id==0 && row==0) mask=0xFFBFu;
                    else mask=0xBFFFu;
                } else {
                    if (id==0 && row<1) mask=0xFFBFu;
                    else if (id==0 && row>1) mask=0xFFEFu;
                    else if (id==0) mask=0xBFFFu;
                    else if (id==8 && row>0) mask=0xFFEFu;
                    else if (id==8 && PE_LoadU32(focus+68u)>0) mask=0xFF7Fu;
                    else if (id==8) mask=0xBFFFu;
                    else if (id==41 && PE_LoadU32(0x8009CFA8u)==0x80046DBCu)
                        mask=PE_LoadU32(focus+68u)>0?0xFF7Fu:0xBFFFu;
                    else if (id==1 || id==2 || id==3 || id==5 || id==7) mask=0xDFFFu;
                    fprintf(stderr,"LOOT_PILOT_HEAL_MENU %d id=%u row=%u pad=%04X\n",g_frame,id,row,mask);
                }
            }
        } else if (!m34 && g_frame%16==3) {
            mask&=0xBFFFu;
        }
        static unsigned last=0x10000u;
        if (mask!=last) {fprintf(stderr,"LOOT_PILOT_PAD %d:%04X\n",g_frame,mask);last=mask;}
    } else {heal_start=-1;equip_start=-1;}
    /* The first victory now happens later than the historical timed suffix.
     * Use the original exit rectangles through normal movement. */
    if (((GA_TOKEN==0xA80023C8u && (g_sewer_victories&1u)) || GA_TOKEN==0xA8002448u) &&
        !(PE_LoadU32(0x8009D1A0u)&2u)) {
        pe_addr_t aya=PE_LoadU32(0x8009D254u);
        mask=0xFFFFu;
        if (PE_LoadU32(0x8009D1A0u)&4u) {
            if (g_frame%16==3) mask=0xDFFFu;
        } else if (aya) {
            int dx=-((int32_t)PE_LoadU32(aya+40u)>>16);
            int dz=(GA_TOKEN==0xA80023C8u?-1250:450)-((int32_t)PE_LoadU32(aya+48u)>>16);
            int c=(int16_t)PE_LoadU16(0x800BD000u),sn=(int16_t)PE_LoadU16(0x800BD004u);
            int x=c*dx-sn*dz,z=sn*dx+c*dz;
            if (dx*dx+dz*dz>1600) {
                if (abs(x)*2>abs(z)) mask&=x>0?0xFFDFu:0xFF7Fu;
                if (abs(z)*2>abs(x)) mask&=z>0?0xFFEFu:0xFFBFu;
            }
        }
    }
    if ((GA_TOKEN==0xA80030C8u || GA_TOKEN==0xA8003148u || GA_TOKEN==0xA80031C8u) && !(PE_LoadU32(0x8009D1A0u)&2u)) {
        pe_addr_t aya=PE_LoadU32(0x8009D254u);
        mask=0xFFFFu;
        if (PE_LoadU32(0x8009D1A0u)&4u) {
            if (g_frame%16==3) mask=0xDFFFu;
        } else if (aya) {
            int ax=(int32_t)PE_LoadU32(aya+40u)>>16,az=(int32_t)PE_LoadU32(aya+48u)>>16;
            int tx,tz;
            if (GA_TOKEN==0xA80030C8u) {
                if (m31_stage==0 && abs(ax)<=40) m31_stage=1;
                if (m31_stage==1 && az>=1000-40) m31_stage=2;
                tx=m31_stage<2?0:-1500;tz=m31_stage==0?az:1000;
            }
            else if (GA_TOKEN==0xA80031C8u) {tx=-1000;tz=0;}
            else if ((g_sewer_victories&4u) && !(PE_LoadU32(0x800A7878u)&1u)) {
                if (m32_stage==0 && az>=3260) m32_stage=1;
                if (m32_stage==1 && ax<=-16930) m32_stage=2;
                tx=m32_stage==0?-16400:-16970;
                tz=m32_stage==0?(ax>-16360?az:3300):m32_stage==1?3300:3520;
            } else {tx=-16400;tz=abs(ax+16400)>40?az:3800;}
            int dx=tx-ax,dz=tz-az;
            int c=(int16_t)PE_LoadU16(0x800BD000u),sn=(int16_t)PE_LoadU16(0x800BD004u);
            int x=c*dx-sn*dz,z=sn*dx+c*dz;
            if (dx*dx+dz*dz>1600) {
                if (abs(x)*2>abs(z)) mask&=x>0?0xFFDFu:0xFF7Fu;
                if (abs(z)*2>abs(x)) mask&=z>0?0xFFEFu:0xFFBFu;
            }
            /* M31's first visit waits at script 8019E600 for dialogue. */
            if(g_frame%16==3)mask&=0xBFFFu;
            if(g_frame%240==0)fprintf(stderr,"REWARD_FORWARD %d token=%08X pos=%d,%d rot=%u persist22=%X persist2B=%X\n",g_frame,GA_TOKEN,ax,az,PE_LoadU16(aya+0x3Au),PE_LoadU32(0x800A7878u),PE_LoadU32(0x800A789Cu));
        }
    }
    static unsigned last_xp=UINT32_MAX,last_base=UINT32_MAX,last_ammo=UINT32_MAX;
    if (g_frame>18500 && g_frame<20500) {
        unsigned xp=PE_LoadU32(0x800C0E00u),base=PE_LoadU32(0x8009D03Cu),ammo=PE_LoadU16(0x800A1E6Eu);
        if (xp!=last_xp || base!=last_base || ammo!=last_ammo) {
            fprintf(stderr,"LOOT_CONNECTED_STATE frame=%d xp=%u base=%u ammo=%u pending=%u\n",g_frame,xp,base,ammo,PE_LoadU32(0x8009D078u));
            last_xp=xp;last_base=base;last_ammo=ammo;
        }
    }
    return mask;
}
