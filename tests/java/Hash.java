import java.security.MessageDigest;
import java.util.ArrayList;
import java.util.List;

import net.jpountz.xxhash.StreamingXXHash32;
import net.jpountz.xxhash.XXHashFactory;

public class XxHashMessageDigest extends MessageDigest {

public static final String XXHASH = "xxhash";
private static final int SEED = 0x9747b28c;
private static final XXHashFactory FACTORY = XXHashFactory
        .fastestInstance();
private List<Integer> values = new ArrayList<Integer>();

protected XxHashMessageDigest() {
    super(XXHASH);
}

@Override
protected void engineUpdate(byte input) {
    // intentionally no implementation
}

@Override
protected void engineUpdate(byte[] input, int offset, int len) {

    StreamingXXHash32 hash32 = FACTORY.newStreamingHash32(SEED);
    hash32.update(input, offset, len);
    values.add(hash32.getValue());
}

@Override
protected byte[] engineDigest() {

    /*
     * TODO provide implementation to "digest" the list of values into a
     * hash value
     */

    engineReset();

    return null;
}

@Override
protected void engineReset() {
    values.clear();
}

}
