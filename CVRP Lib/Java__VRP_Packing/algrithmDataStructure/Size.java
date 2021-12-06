package com.quadtalent.quadx.algrithmDataStructure;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.Objects;


@Data
@NoArgsConstructor
@AllArgsConstructor
public class Size {
    private float lx;
    private float ly;
    private float lz;

    @Override
    public boolean equals(Object obj){
        if(this == obj){    //先判断是否是同一个对象
            return true;
        }

        if(obj == null){    //如果对象为null，返回false
            return false;
        }

        //判断两个对象是否属于同一个类
        if(!(obj instanceof Size)){
            return false;
        }

        Size other = (Size) obj;
        if (lx==other.lx && ly==other.ly && lz==other.lz){
            return true;
        }
        return false;
    }

    @Override
    public int hashCode() {
        return Objects.hash(lx, ly, lz);
    }
}
