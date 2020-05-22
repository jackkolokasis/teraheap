package java.lang;
import java.lang.annotation.*;

@Target(ElementType.TYPE_USE)
@Retention(value=RetentionPolicy.RUNTIME)
public @interface Cache {}
